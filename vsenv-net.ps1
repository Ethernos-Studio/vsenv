param(
    [string]$InstanceName,
    [switch]$RandomHost,
    [switch]$RandomMac,
    [string]$Proxy,
    [switch]$UndoChanges
)

# 加载 NetAdapter 模块
Import-Module NetAdapter -ErrorAction SilentlyContinue

# 检查是否以管理员身份运行
function Check-Admin {
    $currentUser = New-Object Security.Principal.WindowsPrincipal $([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
}

# 提升权限运行脚本
function Invoke-WithAdminRights {
    param(
        [scriptblock]$ScriptBlock
    )
    $currentArgs = $Args
    Start-Process -FilePath "powershell.exe" -Verb RunAs -ArgumentList "-Command & { Invoke-Expression `$using:ScriptBlock }"
    exit
}

# 撤销更改
function Undo-Changes {
    param(
        [string]$InstanceName
    )
    # 恢复主机名（如果修改过）
    if ($env:ORIGINAL_HOSTNAME) {
        Rename-Computer -NewName $env:ORIGINAL_HOSTNAME -Force *>$null
        Remove-Item -Path "env:ORIGINAL_HOSTNAME"
    }

    # 删除虚拟网卡
    Get-NetAdapter | Where-Object { $_.Name -like "VSenv-$InstanceName" } | Remove-NetAdapter -Confirm:$false *>$null

    # 重置代理
    netsh winhttp reset proxy
}

# 主逻辑
if (-not (Check-Admin)) {
    Invoke-WithAdminRights -ScriptBlock {
        param($InstanceName, $RandomHost, $RandomMac, $Proxy, $UndoChanges)
        # 在此执行需要管理员权限的操作
        if ($UndoChanges) {
            Undo-Changes -InstanceName $InstanceName
            Write-Host "所有更改已撤销"
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
                Write-Host "无法创建虚拟网卡，请检查是否以管理员身份运行脚本。"
            }
        }

        if ($Proxy -and (Test-NetConnection -ComputerName 127.0.0.1 -Port ([System.Uri]$Proxy).Port -InformationLevel Quiet)) {
            netsh winhttp set proxy $Proxy
            Write-Host "已启用代理 $Proxy"
        } else {
            netsh winhttp reset proxy
            Write-Host "代理未开启，已恢复直连"
        }
    } -ArgumentList $InstanceName, $RandomHost, $RandomMac, $Proxy, $UndoChanges
    exit
}

if ($UndoChanges) {
    Undo-Changes -InstanceName $InstanceName
    Write-Host "所有更改已撤销"
    exit
}

# 设置网络隔离
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
        Write-Host "无法创建虚拟网卡，请检查是否以管理员身份运行脚本。"
    }
}

if ($Proxy -and (Test-NetConnection -ComputerName 127.0.0.1 -Port ([System.Uri]$Proxy).Port -InformationLevel Quiet)) {
    netsh winhttp set proxy $Proxy
    Write-Host "已启用代理 $Proxy"
} else {
    netsh winhttp reset proxy
    Write-Host "代理未开启，已恢复直连"
}