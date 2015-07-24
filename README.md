# OSCourseProject
###操作系统课程设计项目
* Member:秦博 1352961
* Member:姬翔 1352825
* Member:霍腾 1352901

###开发环境
* Linux Ubuntu 14.04
* bochs 2.6.7
* sublime-text 2


###使用方法
* 确保你已经装了<a href="http://www.ubuntu.com/download/desktop">Ubuntu</a> 和 <a href="http://sourceforge.net/projects/bochs/">Bochs</a>
* 在terminal里面运行下面的命令完成配置，如果是中国IP可能需要切换Ubuntu的源（163的源速度较快）.
```bash
$ sudo  apt-get update 
$ sudo	apt-get	install	build-essential	
$ sudo	apt-get	install	xorg-dev										
$ sudo	apt-get	install	bison	
$ sudo	apt-get	install	libgtk2.0-dev
$ sudo	apt-get	install	nasm
$ sudo	apt-get	install	vgabios
```
* 将项目clone到本地,首先打开到目标文件夹下,然后运行:
```bash
$ git clone https://github.com/xdliu002/OSCourseProject.git
$ cd OSCourseProject /Tinix-master
$ sudo make image
$ bochs -f bochsrc.bxrc
$ 回车再输入 c 进入
```

=====
###项目分工

* 霍腾负责进程调度算法的修改，添加了一个用户级应用猜拳游戏作为一个单独的进程，美化系统界面。
* 姬翔主要添加了一个用户级应用作为单独进程：日历。
* 秦博主要添加了一个用户级应用作为单独进程：2048游戏。并且设计、美化了系统界面。

=======
###托管地址：
https://github.com/xdliu002/OSCourseProject_Qin

之前这个项目是闭源项目，我们三人都没有闭源库，所以该项目托管在了别人的账户下。

====
Huo © 2015.7.23

