ssh root@192.168.157.2 "cd /tmp/; cp /mnt/rtfiles/lib* /lib/; cp /mnt/rtfiles/eni* .; pwd"
scp "C:\Users\fatih\OneDrive\Masaüstü\Acontis-Workspace_Burak\RTM_UserExpression\Debug\libSharedLib.so" root@192.168.157.2:/lib/
scp Debug\RTM_Test root@192.168.157.2:/tmp/
ssh root@192.168.157.2 "chmod 777 /tmp/RTM_Test"