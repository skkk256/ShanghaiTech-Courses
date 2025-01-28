1. Mat文件：rawTracePersonX


- 变量 dataTrial 2100x40x19 时间 * Trial * 通道，包含了各通道各Trail的Rawtrace
   - 时间：包含2100个数据点，对应2100个RawTrace的时间。具体时间在timeRawTrace.mat（1x2100）文件中。采样频率300Hz 2100个点共对应7s。每个人的timeRawTrace都是一样的。
   - Trial：一共40个Trial，有些人是10个Trial。
   - 通道：一共24个通道，此处只包含19个通道[1:8 10:16 19:20 23:24]，不包含9CM 17X3 18X2 21X1 22A2。cm（废弃通道），X1 X2 X3 A2为空通道（什么都没接）。通道的名称在ChanName.mat文件中

- 变量 Track 1x40 
	40个trial对应的图片编号。<11的编号为记忆过的图片，>10的是没有记忆过的。

2. Mat文件：OSPersonX
- 变量：OS：要分析的数据，36x52x40x54 时间 * 频率 * Trial * pair
	- time：对应的时间点在Time变量中
	- 频率：对应的频率在fOS变量中
	- Trial：同样为40个Trail或10个Trial，顺序和Track对应。
	- Pair：这里Pair指的是54个感兴趣的两个通道之间的配对。对应的Pair编号在Pair54.mat文件中。