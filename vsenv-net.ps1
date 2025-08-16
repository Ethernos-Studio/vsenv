param(
    [string]$InstanceName,
    [switch]$RandomHost,
    [switch]$RandomMac,
    [string]$Proxy,
    [switch]$UndoChanges
)

# ���� NetAdapter ģ��
Import-Module NetAdapter -ErrorAction SilentlyContinue

# ����Ƿ��Թ���Ա�������
function Check-Admin {
    $currentUser = New-Object Security.Principal.WindowsPrincipal $([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
}

# ����Ȩ�����нű�
function Invoke-WithAdminRights {
    param(
        [scriptblock]$ScriptBlock
    )
    $currentArgs = $Args
    Start-Process -FilePath "powershell.exe" -Verb RunAs -ArgumentList "-Command & { Invoke-Expression `$using:ScriptBlock }"
    exit
}

# ��������
function Undo-Changes {
    param(
        [string]$InstanceName
    )
    # �ָ�������������޸Ĺ���
    if ($env:ORIGINAL_HOSTNAME) {
        Rename-Computer -NewName $env:ORIGINAL_HOSTNAME -Force *>$null
        Remove-Item -Path "env:ORIGINAL_HOSTNAME"
    }

    # ɾ����������
    Get-NetAdapter | Where-Object { $_.Name -like "VSenv-$InstanceName" } | Remove-NetAdapter -Confirm:$false *>$null

    # ���ô���
    netsh winhttp reset proxy
}

# ���߼�
if (-not (Check-Admin)) {
    Invoke-WithAdminRights -ScriptBlock {
        param($InstanceName, $RandomHost, $RandomMac, $Proxy, $UndoChanges)
        # �ڴ�ִ����Ҫ����ԱȨ�޵Ĳ���
        if ($UndoChanges) {
            Undo-Changes -InstanceName $InstanceName
            Write-Host "���и����ѳ���"
            return
        }

        if ($RandomHost) {
            $env:ORIGINAL_HOSTNAME = $env:COMPUTERNAME
            $HostName = "VS-" + (Get-Random -Minimum 1000 -Maximum 9999)
            Rename-Computer -NewName $HostName -Force *>$null
        }

        if ($RandomMac) {
            $Mac = "02:{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}" -f (1..5 | %{ Get-Random -Min 0 -Max 255 })
            try {
                New-NetAdapter -Name "VSenv-$InstanceName" -MacAddress $Mac -InterfaceAlias "VSenv-$InstanceName" -Confirm:$false -ErrorAction Stop
            } catch {
                Write-Host "�޷��������������������Ƿ��Թ���Ա������нű���"
            }
        }

        if ($Proxy -and (Test-NetConnection -ComputerName 127.0.0.1 -Port ([System.Uri]$Proxy).Port -InformationLevel Quiet)) {
            netsh winhttp set proxy $Proxy
            Write-Host "�����ô��� $Proxy"
        } else {
            netsh winhttp reset proxy
            Write-Host "����δ�������ѻָ�ֱ��"
        }
    } -ArgumentList $InstanceName, $RandomHost, $RandomMac, $Proxy, $UndoChanges
    exit
}

if ($UndoChanges) {
    Undo-Changes -InstanceName $InstanceName
    Write-Host "���и����ѳ���"
    exit
}

# �����������
if ($RandomHost) {
    $env:ORIGINAL_HOSTNAME = $env:COMPUTERNAME
    $HostName = "VS-" + (Get-Random -Minimum 1000 -Maximum 9999)
    Rename-Computer -NewName $HostName -Force *>$null
}

if ($RandomMac) {
    $Mac = "02:{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}" -f (1..5 | %{ Get-Random -Min 0 -Max 255 })
    try {
        New-NetAdapter -Name "VSenv-$InstanceName" -MacAddress $Mac -InterfaceAlias "VSenv-$InstanceName" -Confirm:$false -ErrorAction Stop
    } catch {
        Write-Host "�޷��������������������Ƿ��Թ���Ա������нű���"
    }
}

if ($Proxy -and (Test-NetConnection -ComputerName 127.0.0.1 -Port ([System.Uri]$Proxy).Port -InformationLevel Quiet)) {
    netsh winhttp set proxy $Proxy
    Write-Host "�����ô��� $Proxy"
} else {
    netsh winhttp reset proxy
    Write-Host "����δ�������ѻָ�ֱ��"
}