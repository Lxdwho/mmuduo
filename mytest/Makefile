# test_muduo:
# 	g++ test_muduo.cpp -I/home/me/muduo_ws 							\
# 						-I/home/me/muduo_ws/muduo-master 			\
# 						-L/home/me/muduo_ws/muduo-master/build/lib 	\
# 						-lmuduo_net -lmuduo_base -lpthread 			\
# 						-o test_muduo.out

test_muduo:
	g++ test_muduo.cpp	-I/home/me/muduo_ws/mmuduo					\
						-I/home/me/muduo_ws/mmuduo/mytest			\
						-L/home/me/muduo_ws/mmuduo/lib/				\
						-lmmuduo -lpthread							\
						-o test_muduo.out

test_Timestamp:
	g++ test_Timestamp.cpp ../Timestamp.cpp  	\
		-I/home/me/muduo_ws/mmuduo			 	\
		-I/home/me/muduo_ws/mmuduo/mytest 		\
		-o test_Timestamp.out

test_Logger:
	g++ test_Logger.cpp ../Timestamp.cpp ../Logger.cpp  	\
		-I/home/me/muduo_ws/mmuduo			 				\
		-I/home/me/muduo_ws/mmuduo/mytest 					\
		-o test_Logger.out

test_InetAddress:
	g++ test_InetAddress.cpp ../InetAddress.cpp	\
		-I/home/me/muduo_ws/mmuduo			 				\
		-I/home/me/muduo_ws/mmuduo/mytest 					\
		-o test_InetAddress.out

clean:
	rm -rf *.out

	