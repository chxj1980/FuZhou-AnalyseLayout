objects = XSAnalyseServer.o AnalyseServer.o LibInfo.o FeatureManage.o ConfigRead.o ZBase64.o SearchThread.o ZeromqManage.o
lib = -lpthread -lmysqlclient -I/usr/local/include/mysql -L/usr/local/lib/mysql -L/usr/local/lib/st -lst_feature_comp -lzmq

XSAnalyseServer: $(objects)
	g++ -o XSAnalyse_Layout $(objects) $(lib)
XSAnalyseServer.o: XSAnalyseServer.cpp AnalyseServer.h
	g++ -c  XSAnalyseServer.cpp 
AnalyseServer.o: AnalyseServer.cpp AnalyseServer.h DataDefine.h LibInfo.h
	g++ -c AnalyseServer.cpp
LibInfo.o: LibInfo.cpp LibInfo.h FeatureManage.h DataDefine.h ZBase64.h
	g++ -c LibInfo.cpp
FeatureManage.o: FeatureManage.cpp FeatureManage.h
	g++ -c FeatureManage.cpp
SearchThread.o: SearchThread.cpp SearchThread.h
	g++ -c SearchThread.cpp
ZeromqManage.o: ZeromqManage.cpp ZeromqManage.h DataDefine.h
	g++ -c ZeromqManage.cpp
ConfigRead.o: ConfigRead.cpp ConfigRead.h
	g++ -c ConfigRead.cpp
ZBase64.o: ZBase64.cpp ZBase64.h
	g++ -c ZBase64.cpp
	
	
clean:
	rm XSAnalyse_Layout $(objects)

cpp:
	rm *.cpp *.h rapidjson/*.h rapidjson/error/*.h rapidjson/internal/*.h	rapidjson/msinttypes/*.h STLib/libst_feature_comp.so STLib/feature_util 
