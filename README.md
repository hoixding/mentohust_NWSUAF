# mentohust_NUSUAF

在参考下方教程后，经过抓包与修改数据，拼凑出可用的锐捷，在主流Linux发行版下测试可用，openwrt也可用。

初次运行将mentuhust目录复制到etc目录下

使用命令：

mentohust -u 学号 -p 密码 -n 网卡名 -i 0.0.0.0 -m 255.255.255.0 -g 0.0.0.0 -s 0.0.0.0 -t 8 -e 1 -r 15 -a 1 -d 0 -b 2 -v 6.01 -f /etc/mentohust/DataPackA.mpf -c dhclient -w 

之后只用mentohust即可

目前存在的问题：

- 校园网不计流量，使用本客户端等于白嫖校园网（所以不敢用，希望可以改进）
- linux下认证成功后，要手动dhcp获取ip地址

再次声明！！！！！！！！！！！！！！！！！

再次声明！！！！！！！！！！！！！！！！！

再次声明！！！！！！！！！！！！！！！！！

#权责声明
1、本程序所有涉及锐捷赛尔认证的功能均是来自前辈公开代码及抓包分析。
2、本程序于个人仅供学习，于他人仅供方便认证，不得使用本程序有意妨害锐捷赛尔认证机制及相关方利益。
3、一切使用后果由用户自己承担。
4、本程序不提供任何服务及保障，编写及维护纯属个人爱好，随时可能被终止。
5、使用本程序者，即表示同意该声明。谢谢合作。

------



使用之前，请查看 http://codingstory.com.cn/mo-gai-mentohust-v4ban-ben-de-xin-de/ 中的介绍，自行修改代码后使用。该版本只有我自己能用，只有我自己能用，只有我自己能用，我做不出通用的，不好意思。

用 Mentohust 最初版的源代码加上 Fork 来的V4算法拼凑出的一个Mentohust.

适用于集美大学教育网接入学生

Ubuntu下实测认证成功

其他VPN服务更新中



--------------------------------------------------------------------------------

完全没怎么看原来的代码，瞎改的。
加入了V4支持。具体算法看checkV4.c

所有hash都是从算法在rhash和ampheck借来魔改的……。

建议Arch Linux用户安装AUR中的mentohust-git
#使用方法

#安装mentohust：建议Ubuntu用户使用Deb包安装，Fedora用户使用RPM包安装

#如果确定自己可以使用xrgsu认证成功，打开终端输入sudo mentohust运行即可
如果不确定，切换到锐捷所在目录，然后输入以下命令：
sudo mkdir /etc/mentohust
sudo cp ./8021x.exe  /etc/mentohust
sudo cp ./W32N55.dll /etc/mentohust
然后打开终端输入sudo mentohust运行。如果认证失败，再切换到锐捷所在目录，输入以下命令：
sudo cp ./SuConfig.dat /etc/mentohust
然后打开终端输入sudo mentohust运行即可。
如果准确按以上步骤操作后还是认证失败，请下载MentoHUSTTool，在Windows下抓包并保存为data.mpf，
然后回到Linux，切换到data.mpf所在目录，输入以下命令：
sudo cp ./data.mpf /etc/mentohust
然后打开终端输入sudo mentohust -f/etc/mentohust/data.mpf -w运行即可。以后也只需输入sudo mentohust。

#您也可以按下面的方法操作：
1、静态IP用户请事先设置好IP；
2、打开终端，输入sudo mentohust，回车；
3、［正确］输入相应信息，如果认证成功，跳到第8步；如果提示“不允许使用的客户端类型”，按Ctrl+C结束认证；
4、打开终端，输入sudo mentohust -w -f'锐捷目录下任意文件路径'，回车；
5、如果认证成功，跳到第8步；如果提示“客户端完整性被破坏”，按Ctrl+C结束认证；
6、将锐捷安装目录下的SuConfig.dat重命名为其他名字；
7、打开终端，输入sudo mentohust，回车；
8、如果是动态IP且不是Linux，打开相应设置去更新IP。
9、以后认证只需打开终端，输入sudo mentohust，回车。
10、要修改某些参数请输入mentohust -h查看帮助信息并据此修改，例如修改密码sudo mentohust -pNewPassword -w，要临时修改则不加-w参数。

#如何退出:不以后台模式运行mentohust时，按Ctrl+C即可退出；后台运行时使用sudo mentohust -k退出认证。

#查看帮助信息请输入：mentohust -h
更详细的帮助信息请参考：http://wiki.ubuntu.org.cn/锐捷、赛尔认证MentoHUST

#修改参数请根据帮助信息操作，例如修改用户名和密码：sudo mentohust -uUsername -pPassword -w
指定某些参数仅对当次认证有效请根据帮助信息操作，例如临时修改用户名和密码：sudo mentohust -uUsername -pPassword

#如果提示缺少libpcap.so.0.x而在/usr/lib/目录下已存在一个libpcap.so.0.x.y，输入以下命令：
sudo ln -s libpcap.so.0.x.y /usr/lib/libpcap.so.0.x
否则请安装libpcap。

#权责声明
1、本程序所有涉及锐捷赛尔认证的功能均是来自前辈公开代码及抓包分析。
2、本程序于个人仅供学习，于他人仅供方便认证，不得使用本程序有意妨害锐捷赛尔认证机制及相关方利益。
3、一切使用后果由用户自己承担。
4、本程序不提供任何服务及保障，编写及维护纯属个人爱好，随时可能被终止。
5、使用本程序者，即表示同意该声明。谢谢合作。

源码可在项目主页获取：http://mentohust.googlecode.com/
联系作者：在http://mentohust.googlecode.com/留言或Email：mentohust@ehust.co.cc
