# vsenv-stop.ps1
param(
    [Parameter(Mandatory=$true)]
    [string]$InstanceName
)

# 需要管理员？
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)) {
    Start-Process powershell.exe -Verb RunAs -ArgumentList @(
        '-NoLogo','-NoProfile','-ExecutionPolicy','Bypass',
        '-File', $MyInvocation.MyCommand.Path,
        '-InstanceName', $InstanceName
    )
    exit
}

# 恢复主机名
if (Test-Path env:ORIGINAL_HOSTNAME) {
    Rename-Computer -NewName $env:ORIGINAL_HOSTNAME -Force *>$null
    Remove-Item -Path env:ORIGINAL_HOSTNAME
}

# 清理虚拟网卡
Get-NetAdapter | Where-Object { $_.Name -like "VSenv-*$InstanceName" } |
    Remove-NetAdapter -Confirm:$false *>$null

# 重置代理
netsh winhttp reset proxy