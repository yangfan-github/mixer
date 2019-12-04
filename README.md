# mixer
my English is poor so 下面全是中文   
这个项目是一个多路视音频混合转码项目，基于boost，ffmpeg，没有用到任何系统相关API，理论上可跨平台.   

编译  
1.编译boost 1.7 或以上版本      输出到系统目录   
2.编译ffmpeg ./thrd-party/ffSDK  输出到系统目录  
3.make    编译并自动输出到系统目录     

目录结构
./media/                      //多媒体框架项目   
./mixerEngine/                //混合转码项目   
./import/                     //转推项目 
./ffmpeg/                     //ffmpeg插件项目  
./mixerTest/                  //混合转码测试项目  
./importTest/                 //转推测试项目  
./lib/libmedia.so             //放置在系统目录  
./lib/libmediad.so            //放置在系统目录  
./lib/libmixerEngine.so       //放置在系统目录  
./lib/libmixerEngined.so      //放置在系统目录  
./lib/libimport.so            //放置在系统目录  
./lib/libimportd.so           //放置在系统目录  
./bin/plugins/libffmpeg.so    //ffmpeg插件  
./bin/plugins/libffmpegd.so   //ffmpeg插件Debug版本  
./bin/mixerTest               //混合转码测试程序  
./bin/mixerTestd              //混合转码测试程序Debug版本   
./bin/importTest              //转推测试程序  
./bin/importTestd             //转推测试程序Debug版本   
./bin/tempalte.json           //转码转码模板   
./bin/template/               //转码转码模板资源目录    
./bin/task.json               //转码任务    
./bin/task/                   //转码任务资源目录   
./inc  //头文件目录    
   
测试程序   
cd ./bin   
./mixerTestd ./template.json ./task.json    //转码测试，输出./out.mp4    
./importTestd [rtmp pull url] [rtmp push url] //转推测试     
