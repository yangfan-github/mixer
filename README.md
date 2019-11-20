# mixer
my English is poor so 下面全是中文
这个项目是一个多路视音频混合转码项目，基于boost，ffmpeg，没有用到任何系统相关API，理论上可跨平台
./bin/out.mp4 是转码输出样例视频
./bin/mixerTest 测试主程序
./bin/template.json 模版文件
./bin/task.json 任务文件
./bin/template/ 模版资源文件夹
./bin/task/ 任务输入文件
./ffmpeg/ ffmpeg插件项目目录
./media/  多媒体框架项目目录
./mixerEngine/ 转码引擎项目目录
./mixerTester/ 转码测试主程序项目目录

依赖以及层次关系

mixerTester
     |
mixerEngine
     |
   media
     |
   ffmpeg
     
     
