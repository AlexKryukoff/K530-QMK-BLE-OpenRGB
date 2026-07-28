<#
    K530 auto-rescan watcher for Skydimo (ASCII only - safe for PowerShell 5.1)

    What it does:
      - waits for a USB device-arrival event from Windows
      - checks that the arrived device is the Redragon K530 (VID_0C45 / PID_5004)
      - sends a single JSON-RPC "scan_devices" command to Skydimo Core

    It does NOT restart Skydimo, so other devices and their effects stay untouched.

    The Core RPC port is discovered dynamically every time, because Skydimo
    picks a fresh port on each start (CORE_PORT). Nothing is hardcoded.

    Expected location:
      C:\Users\alexk\AppData\Roaming\com.skydimo.desktop\plugins\k530-rescan-watcher.ps1

    The log file is written next to this script (k530-rescan.log).

    Manual run (for testing, keeps a console window open):
      powershell -ExecutionPolicy Bypass -File "$env:APPDATA\com.skydimo.desktop\plugins\k530-rescan-watcher.ps1"

    Autostart: run install-k530-task.ps1 from the same folder.
#>

[CmdletBinding()]
param(
    # USB ids of the keyboard, as they appear in Windows PNPDeviceID
    [string]$VidPid = 'VID_0C45&PID_5004',

    # How long to wait after arrival before scanning, so Windows can finish
    # mounting all HID interfaces (the plugin needs interface_number 1).
    # Increase this if the first scan sometimes misses the keyboard.
    [int]$SettleSeconds = 4,

    # Ignore repeated arrival events within this window (one replug fires many).
    [int]$DebounceSeconds = 8,

    # Empty = put the log next to this script.
    [string]$LogPath = ''
)

$ErrorActionPreference = 'Continue'

# Resolve the log path relative to the script location, so everything
# (script + log) stays in one folder.
if ([string]::IsNullOrWhiteSpace($LogPath)) {
    $baseDir = $PSScriptRoot
    if ([string]::IsNullOrWhiteSpace($baseDir)) { $baseDir = (Get-Location).Path }
    $LogPath = Join-Path $baseDir 'k530-rescan.log'
}

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
function Write-Log {
    param([string]$Message, [string]$Level = 'INFO')

    $line = '{0} [{1}] {2}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), $Level, $Message
    Write-Host $line

    try {
        $dir = Split-Path -Parent $LogPath
        if ($dir -and -not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }

        # keep the log small
        if ((Test-Path $LogPath) -and ((Get-Item $LogPath).Length -gt 512000)) {
            Remove-Item $LogPath -Force -ErrorAction SilentlyContinue
        }
        Add-Content -Path $LogPath -Value $line -Encoding UTF8 -ErrorAction SilentlyContinue
    }
    catch { }
}

# ---------------------------------------------------------------------------
# JSON-RPC over WebSocket
# ---------------------------------------------------------------------------
function Invoke-SkyRpc {
    param(
        [Parameter(Mandatory)][int]$Port,
        [Parameter(Mandatory)][string]$Method,
        [hashtable]$Params,
        [int]$TimeoutSec = 15
    )

    $ws  = New-Object System.Net.WebSockets.ClientWebSocket
    $cts = New-Object System.Threading.CancellationTokenSource([TimeSpan]::FromSeconds($TimeoutSec))

    try {
        $ws.ConnectAsync([Uri]"ws://127.0.0.1:$Port", $cts.Token).GetAwaiter().GetResult()
    }
    catch {
        return [pscustomobject]@{ Ok = $false; Stage = 'connect'; Error = $_.Exception.Message }
    }

    try {
        $req = @{ jsonrpc = '2.0'; method = $Method; id = 1 }
        if ($Params) { $req['params'] = $Params }

        $json  = $req | ConvertTo-Json -Depth 8 -Compress
        $bytes = [Text.Encoding]::UTF8.GetBytes($json)
        $seg   = New-Object System.ArraySegment[byte](, $bytes)

        $ws.SendAsync($seg, [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $cts.Token).GetAwaiter().GetResult()

        $buf      = New-Object byte[] 131072
        $sb       = New-Object System.Text.StringBuilder
        $deadline = (Get-Date).AddSeconds($TimeoutSec)

        while ((Get-Date) -lt $deadline) {
            $rseg = New-Object System.ArraySegment[byte](, $buf)
            $r    = $ws.ReceiveAsync($rseg, $cts.Token).GetAwaiter().GetResult()

            if ($r.MessageType -eq [System.Net.WebSockets.WebSocketMessageType]::Close) { break }

            [void]$sb.Append([Text.Encoding]::UTF8.GetString($buf, 0, $r.Count))
            if (-not $r.EndOfMessage) { continue }

            $text = $sb.ToString()
            [void]$sb.Clear()

            $obj = $null
            try { $obj = $text | ConvertFrom-Json } catch { continue }

            # Core also pushes events (no id field) - skip them, wait for id = 1
            if ($null -ne $obj.PSObject.Properties['id'] -and $obj.id -eq 1) {
                if ($obj.PSObject.Properties['error']) {
                    return [pscustomobject]@{ Ok = $false; Stage = 'rpc'; Error = ($obj.error | ConvertTo-Json -Compress) }
                }
                return [pscustomobject]@{ Ok = $true; Result = $obj.result }
            }
        }

        return [pscustomobject]@{ Ok = $false; Stage = 'timeout'; Error = "no reply within $TimeoutSec s" }
    }
    finally {
        try { $ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $cts.Token).GetAwaiter().GetResult() } catch { }
        $ws.Dispose()
    }
}

# ---------------------------------------------------------------------------
# Find the Core RPC port (it changes on every Skydimo start)
# ---------------------------------------------------------------------------
function Get-SkydimoRpcPort {
    $procIds = @(Get-Process *skydimo* -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
    if ($procIds.Count -eq 0) {
        Write-Log 'Skydimo is not running - nothing to do.' 'WARN'
        return $null
    }

    $ports = @(
        Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
            Where-Object { $procIds -contains $_.OwningProcess -and $_.LocalAddress -eq '127.0.0.1' } |
            Select-Object -ExpandProperty LocalPort -Unique
    )

    if ($ports.Count -eq 0) {
        Write-Log 'Skydimo is running but has no local listening ports.' 'WARN'
        return $null
    }

    foreach ($p in $ports) {
        $probe = Invoke-SkyRpc -Port $p -Method 'get_system_info' -TimeoutSec 5
        if ($probe.Ok) {
            Write-Log ("RPC port found: {0}" -f $p)
            return $p
        }
    }

    Write-Log ("None of the ports [{0}] answered JSON-RPC." -f ($ports -join ', ')) 'WARN'
    return $null
}

# ---------------------------------------------------------------------------
# Is the keyboard actually present right now?
# ---------------------------------------------------------------------------
function Test-KeyboardPresent {
    try {
        $found = Get-CimInstance -ClassName Win32_PnPEntity -Filter "PNPDeviceID LIKE '%$VidPid%'" -ErrorAction Stop
        return ($null -ne $found)
    }
    catch {
        Write-Log ("PnP query failed: {0}" -f $_.Exception.Message) 'WARN'
        return $false
    }
}

# ---------------------------------------------------------------------------
# One rescan attempt, with retries
# ---------------------------------------------------------------------------
function Invoke-Rescan {
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        $port = Get-SkydimoRpcPort
        if (-not $port) {
            Start-Sleep -Seconds 3
            continue
        }

        $scan = Invoke-SkyRpc -Port $port -Method 'scan_devices' -TimeoutSec 30
        if ($scan.Ok) {
            Write-Log 'scan_devices OK - Skydimo asked to re-enumerate.'
            return $true
        }

        Write-Log ("scan_devices failed (attempt {0}/3) [{1}]: {2}" -f $attempt, $scan.Stage, $scan.Error) 'WARN'
        Start-Sleep -Seconds 3
    }

    Write-Log 'Giving up on this arrival event.' 'ERROR'
    return $false
}

# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------
$src = 'K530DeviceArrival'

Get-EventSubscriber -SourceIdentifier $src -ErrorAction SilentlyContinue | Unregister-Event -Force -ErrorAction SilentlyContinue

# EventType 2 = device arrival. This is far lighter than polling
# __InstanceCreationEvent on Win32_PnPEntity, which re-enumerates every device.
$query = 'SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2'

try {
    Register-WmiEvent -Query $query -SourceIdentifier $src -ErrorAction Stop
}
catch {
    Write-Log ("Cannot subscribe to device events: {0}" -f $_.Exception.Message) 'ERROR'
    exit 1
}

Write-Log '========================================'
Write-Log ("Watcher started. Looking for {0}" -f $VidPid)
Write-Log ("Log file: {0}" -f $LogPath)

$lastRun = [DateTime]::MinValue

try {
    while ($true) {
        $ev = Wait-Event -SourceIdentifier $src
        Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue

        # a single replug fires a burst of events - swallow the rest
        Get-Event -SourceIdentifier $src -ErrorAction SilentlyContinue | Remove-Event -ErrorAction SilentlyContinue

        $since = (Get-Date) - $lastRun
        if ($since.TotalSeconds -lt $DebounceSeconds) { continue }

        if (-not (Test-KeyboardPresent)) { continue }

        $lastRun = Get-Date
        Write-Log 'K530 arrival detected.'

        Start-Sleep -Seconds $SettleSeconds

        # drop events that piled up while we were sleeping
        Get-Event -SourceIdentifier $src -ErrorAction SilentlyContinue | Remove-Event -ErrorAction SilentlyContinue

        [void](Invoke-Rescan)
        $lastRun = Get-Date
    }
}
finally {
    Get-EventSubscriber -SourceIdentifier $src -ErrorAction SilentlyContinue | Unregister-Event -Force -ErrorAction SilentlyContinue
    Write-Log 'Watcher stopped.'
}
