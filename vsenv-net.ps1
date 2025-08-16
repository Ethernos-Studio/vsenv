param(
    [string]$InstanceName,
    [switch]$RandomHost,   # 手动开关
    [switch]$RandomMac,    # 手动开关
    [string]$Proxy         # 为空即直连
)

# 1. 随机主机名
if ($RandomHost) {
    $HostName = "VS-" + (Get-Random -Minimum 1000 -Maximum 9999)
    Rename-Computer -NewName $HostName -Force *>$null
}

# 2. 随机 MAC + TAP
if ($RandomMac) {
    $Mac = "02:{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}" -f (1..5 | %{ Get-Random -Min 0 -Max 255 })
    New-NetAdapter -Name "VSenv-$InstanceName" -MacAddress $Mac -InterfaceAlias "VSenv-$InstanceName" -Confirm:$false *>$null
}

# 3. 代理（仅当显式提供且端口可达时）
if ($Proxy -and (Test-NetConnection -ComputerName 127.0.0.1 -Port ([System.Uri]$Proxy).Port) -eq $true) {
    netsh winhttp set proxy $Proxy
} else {
    netsh winhttp reset proxy
}