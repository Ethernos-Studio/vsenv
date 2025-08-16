# vsenv-stop.ps1
param(
    [Parameter(Mandatory=$true)]
    [string]$InstanceName
)

# ��Ҫ����Ա��
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)) {
    Start-Process powershell.exe -Verb RunAs -ArgumentList @(
        '-NoLogo','-NoProfile','-ExecutionPolicy','Bypass',
        '-File', $MyInvocation.MyCommand.Path,
        '-InstanceName', $InstanceName
    )
    exit
}

# �ָ�������
if (Test-Path env:ORIGINAL_HOSTNAME) {
    Rename-Computer -NewName $env:ORIGINAL_HOSTNAME -Force *>$null
    Remove-Item -Path env:ORIGINAL_HOSTNAME
}

# ������������
Get-NetAdapter | Where-Object { $_.Name -like "VSenv-*$InstanceName" } |
    Remove-NetAdapter -Confirm:$false *>$null

# ���ô���
netsh winhttp reset proxy