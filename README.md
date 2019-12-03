# mixer
my English is poor so 下面全是中文   
这个项目是一个多路视音频混合转码项目，基于boost，ffmpeg，没有用到任何系统相关API，理论上可跨平台.   

编译  
1.编译boost 1.7 或以上版本      输出到系统目录   
2.编译ffmpeg ./thrd-party/ffSDK  输出到系统目录  
3.make    编译并自动输出到系统目录     

输出  
./lib/libmedia.so          //放置在系统目录  
./lib/libmediad.so         //放置在系统目录  
./lib/libmixerEngine.so    //放置在系统目录  
./lib/libmixerEngined.so   //放置在系统目录  
./lib/libimport.so         //放置在系统目录  
./lib/libimportd.so        //放置在系统目录  
./bin/libffmpeg.so         //ffmpeg插件           必须存放在主程序工作目录  
./bin/libffmpegd.so        //ffmpeg插件Debug版本  必须存放在主程序工作目录  
./bin/mixerTest            //混合转码测试程序  
./bin/mixerTestd           //混合转码测试程序Debug版本   
./bin/importTest           //转推测试程序  
./bin/importTestd          //转推测试程序Debug版本   
 
./inc  //头文件目录  

