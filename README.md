# MagicBox
本项目移植了[ClimbSnail]大佬开发的AIO框架，将其运行在独立开发的硬件之上。
在ClimbSnail的APP框架中添加了系统时间模块，以及温湿度，光照传感器模块

此外，本固件代码完全开源，共大家学习、把玩，若使用本项目二次开源或部分参考，请适当注明参考来源。

* 原作者的项目链接 https://github.com/ClimbSnail/HoloCubic_AIO
* 或者 https://gitee.com/ClimbSnailQ/HoloCubic_AIO
* 本项目连接 https://github.com/ClimbSnail/HoloCubic_AIO

### 主要特点
1. 聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、电脑投屏、web设置、软路由监控等等。（各APP具体使用参考说明书）
2. 开机无论是否插接tf卡、mpu6050是否焊接正常、是否连接wifi（一定要2.4G的wifi），都不影响系统启动和屏幕显示。
3. 程序相对模块化，低耦合。
4. 提供web界面进行配网以及其他设置选项。注：具体操作参考`APP介绍`
5. 提供web端连入除了支持ip访问，也支持域名直接访问 http://holocubic （部分浏览器可能支持不好）
6. 提供web端的文件上传到SD卡（包括删除），无需拔插SD来更新图片。
7. 可使用[ClimbSnail]大佬的上位机软件，并开源上位机源码。 https://github.com/ClimbSnail/HoloCubic_AIO_Tool


### 开机注意事项
由于小电视使用的是MPU6050陀螺仪加速度计，通电前3秒需要保持小电视自然放置（不要手拿），等待传感器初始化，初始化完毕后RGB灯会完全亮起，之后就可以正常操作了。插不插内存卡都不影响正常开机，如果6050焊接有问题，初始化后姿态读取会错乱（现象：应用会不断切换）。

### 功能切换说明：
1. TF卡的文件系统为fat32。TF为非必须硬件，但相册、视频播放等功能需依赖与此。如果准备使用内存卡，在使用内存卡前最好将本工程中`放置到内存卡`目录里的所有文件和文件夹都放在TF卡的根目录。
2. 插不插tf内存卡都不影响开机，但影响某些APP的功能（各自APP介绍里会说明）。
3. 左右摇晃`0.5s`即可切换选择各类APP。
4. 向前倾斜`1s`钟即可进入当前页的APP应用，同样后仰1s即退出该APP。

### APP介绍
未装载的APP可在src/app/app_conf.h中开启

##### 设置
1. 运行条件：无。注：wifi等信息是保存在flash中，内存卡完全不影响wifi功能的连接。
2. 启用后，小电视开启AP模式，AP模式的热点名为`MagicBox_AP`无密码。
3. 开始使用时，应让电脑与`MagicBox`处于同一网络环境（同网段）。如果之前没连接过wifi则需要使用电脑连接MagicBox放出的热点名为`MagicBox_AP`无密码的wifi。
4. 在浏览器地址栏输入`MagicBox.local`或者 屏幕显示的IP地址（ http://192.168.4.2 也支持域名直接访问 MagicBox.local ），即可进入管理设置后台。推荐使用`ip地址`访问。
5. 网页里可设置系统参数、天气APP参数、相册参数、播放器参数等等。

##### 文件管理器（File Manager）（暂未装载）
作用：通过无线网络管理内存卡上的文件。

1. 运行APP条件：必须是已经正常配置wifi。必须插内存卡。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。目前部分功能还在开发中。
2. 进入`MagicBox`文件管理器后会自动连接已配置的wifi，并显示出IP地址。
3. 未完成：在上位机的文件管理器软件中填入自己`MagicBox`的IP地址（端口可以不用改），点击连接。

注：目前文件管理器临时使用`windows资源管理器`，在地址栏输入 ftp://holocubic:aio@192.168.123.241 （192.168.123.241为我的小电视上显示的IP地址，如果提示开启访问，就开启）

##### 相册（Picture）
1. 运行APP条件：必须插内存卡，内存卡的根目录下必须存在`image/`目录（也可以使用`设置`APP 通过浏览器上传照片），`image/`目录下必须要有图片文件（jpg或者bin）。
2. 将需要播放的图片转化成一定格式（.jpg或.bin），再保存在`image/`目录中，图片文件名必须为英文字符或数字。
3. 使用固件进入相册APP后，将会读取`image/`目录下的图片文件。
4. `设置`的网页端可以进行附加功能的设置。

关于图片转换：使用附带的上位机转化（分辨率随意，软件会自动压缩到指定分辨率）。
* 不常用的图片则可以转换成（True color with alpha 选择Binary RGB565）bin文件存储到SD卡中，这样可以省下一些程序存储空间用来增加功能。支持转化为jpg图片。

##### 视频播放（Media）（暂未装载）
1. 运行APP条件：必须插内存卡，内存卡的根目录下必须存在`movie/`目录。
2. 将所需要播放的视频（最好长宽比是1:1），使用本固件配套的使用转化工具转化为目标文件（mjpeg或者rgb格式都可），存放在`movie/`目录下，视频文件名必须为英文字符或数字。
3. 运行播放器APP后，将会读取`movie/`目录下的视频文件。
4. 默认功率下，无任何动作90s后进入低功耗模式，120s后进入二级低功耗模式，具体表现为播放画面帧数下降。
5. `设置`的网页端可以进行附加功能的设置。

##### 屏幕分享、电脑投屏（Screen share）
1. 运行APP条件：无需内存卡，但需要利用`设置`app设置wifi密码（确保能连上路由器）。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. 上位机目前使用第三方软件，后期会独立编写投屏上位机，提高性能。
3. 本投屏上位机使用的是[大大怪](https://gitee.com/superddg123/esp32-TFT/tree/master)的上位机，如果画面卡顿可以降低画质来提升帧数。
4. `设置`的网页端可以进行附加功能的设置。

##### 天气、时钟（Weather）
1. 使用https://www.tianqiapi.com 天气API。
2. 运行APP条件：必须是已经联网状态，且设置好`tianqi_appid`、`tianqi_appsecret`、`tianqi 城市名（中文）`。
3. 使用新版天气时钟，需要再"设置"网页服务中修改`tianqi_appid`、`tianqi_appsecret`。（申请地址 https://www.yiketianqi.com/user/login ）
4. 显示环境温度、湿度
5. 增加3天天气情况页面

##### 特效动画（Idea）
1. 运行APP条件：无。内置的几种特效动画。

注：移植群友"小飞侠"的功能，在此感谢！

##### 文字雨
1. 运行APP条件：无。

##### 2048 APP
1. `2048`游戏由群友`AndyXFuture`编写并同意，由`ClimbSnail`合入AIO固件。原项目链接为`https://github.com/AndyXFuture/HoloCubic-2048-anim`
2. 运行APP条件：无。基本屏幕能亮就行。
3. 操作注意：游戏中`向上`和`向下`操作由于与原`进入`和`退出`为同一个动作，系统已操作时长为区分动作，游戏中`向上`和`向下`正常操作即可，`进入`和`退出`需要倾斜1秒中方可触发。

##### BiliBili APP
1. 运行APP条件：内存卡中必须要有名为`bilibili`的文件夹。必须是已经正常配置wifi。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. `UID`查看方法：电脑浏览器上打开B站并登入账号，之后浏览器打开一个空白页粘贴回车这个网址 https://space.bilibili.com/ ，网址尾巴会自动多出一串纯数字码，此即为UID。
3. 第一次使用之前，要先在`设置 App`的网页上填写`UID`码。
4. 需要在内存卡中名为`bilibili`的文件夹里添加一张名为`avatar.bin`自己B站头像的图片，分辨率为`100*100`的`bin`文件（可以使用AIO上位机转换）。

注：程序由`cnzxo`编写。

##### 纪念日（Anniversary）（暂未装载）
1. 运行APP条件：联网状态
2. 第一次使用之前，要先在`设置 App`的网页上填写纪念日名称和日期，目前可以设置两个纪念日。纪念日支持的字有`生日还有毕业养小恐龙种土豆老婆女朋友爸妈爷奶弟妹兄姐结婚纪念`，如果纪念日名称包含的字不在这个范围内，请自行生成字体文件并替换`src\app\anniversary\msyhbd_24.c`文件。日期格式如`2022.5.8`，如果年份设置为0，则被认为是每年重复的纪念日（如生日）。

注：纪念日和心跳都复现自[LizCubic](https://github.com/qingehao/LizCubic)项目。程序由`WoodwindHu`编写

##### 心跳（Heartbeat）（暂未装载）
1. 运行APP条件：联网状态（需要开启性能模式），一个开放1883端口的mqtt服务器，两个MagicBox。
2. 第一次使用之前，要先在`设置 App`的网页上填写配置。role可以选择0和1，分别代表互动的两个MagicBox。client_id为设备的唯一标识，这里请将这两个MagicBox设置成同一个QQ号。mqtt_server填写自己的mqtt服务器地址,port填写端口号。用户名以及密码根据具体的服务器配置而定。
3. 设置完心跳APP之后，开机自动联网，并开启mqtt客户端。收到另一个MagicBox的消息之后自动进入APP。正常方式进入APP则自动向另一个MagicBox发送消息。
4. 群内不定时更新免费的服务，具体配置参数可以问管理或者群友。

注：纪念日和心跳都复现自[LizCubic](https://github.com/qingehao/LizCubic)项目。程序由`WoodwindHu`编写

##### 股票行情实时查看（Stock）
1. 运行APP条件：必须是已经正常配置wifi。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. 第一次使用之前，要先在`设置 App`的网页上修改自己想要的`股票代码`。

注：程序由`redwolf`编写

##### PC资源监控(PC Resource)
1. 运行条件: 必须是已经正常配置wifi。PC端与设置Magic B o x处于同一网段，在`设置 APP`中这只PC的服务IP地址（具体看教程）。
2. 下载[AIDA64](https://www.aida64.com/downloads)，PC安装AIDA64后的导入配置文件`aida64_setting.rslcd`（在`AIO_Firmware_PIO\src\app\pc_resource`目录下或者群文件中）

##### 路由器监控
1. 运行条件: 必须是已经正常配置wifi。路由器必须安装NetData
2. 第一次使用之前，要先在`设置 App`的网页上填写配置路由器ip地址，端口号一般默认就可以。
3. 如果信息显示不正常，可修改更具实际情况修改Chart ID
##FAQ
1. 烧录后温度信息不显示
参考资料： https://hiwbb.com/2021/10/openwrt-netdata-show-temperature/

原因：
    1. netdata.conf 中关闭了插件chart的显示
    2. 基础软件 coreutils-timeout未安装
解决办法：

    1. 登录openwrt终端
    2. 安装timeout：opkg install coreutils-timeout
    3. 进入/etc/netdata
    4. 使用./edit-config charts.d.conf来编辑配置文件，这个edit-config等于是一个配置工具能够从/usr/lib拉取默认配置过来，在配置里最后加入sensors=force。不加一定不会有温度，原因未知。
    5. 用/usr/lib/netdata/plugins.d/charts.d.plugin sensors测一下，如果有一直跳数据出来，就说明成功了。
    6. Openwrt的版本默认可能关闭了chart.d插件，编辑/etc/netdata/netdata.conf把charts.d = no改为charts.d = yes或直接注释掉那一行，若没有这行则不需要
    7. 重启netdata： /etc/init.d/netdata restart
2. Netdata温度显示正常，但是monitor依旧不显示
如果netdata已经能够正常显示温度，大概率是因为 monitor 请求的key不对，不同的系统版本，温度sensor对应的key存在差异，修改方式如下. 从Netdata中找到温度曲线的key，在‘设置APP’中填写。



