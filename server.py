'''
Author: backlory's desktop dbdx_liyaning@126.com
Date: 2024-03-22 15:21:54
LastEditors: backlory's desktop dbdx_liyaning@126.com
LastEditTime: 2024-03-27 22:33:40
Description: 

基于socket的图像传输服务端。
1. 服务端接收图像数据，解析json数据和图像数据。
2. 显示图像。

Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
'''
#!/usr/bin/python
#-*-coding:utf-8 -*-
import socket
import cv2
import numpy as np
import json


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
address = ('127.0.0.1', 8998)
s.bind(address) # 将Socket（套接字）绑定到地址
s.listen(True) # 开始监听TCP传入连接

conn, addr = s.accept()
temp = conn.recv(6).decode()
conn.send('hello\0'.encode())
print('connected by', addr)

idx = 0
while 1:
    # 接收数据，最大接收长度为200KB
    realLength = 200*1024
    stringData = conn.recv(realLength)
    if not stringData:
        break
    temp = 'OK'+stringData[:32].decode() + "\0"
    conn.send(temp.encode())
    
    # 解析数据
    sizeJson = int(stringData[:16])
    sizeFrame = int(stringData[16:32])
    dataJson = stringData[32:32+sizeJson]
    dataFrame = stringData[32+sizeJson:32+sizeJson+sizeFrame]
    # pose
    dataJson = json.loads(dataJson)
    # frame
    dataFrame = np.frombuffer(dataFrame, dtype='uint8')
    decimg=cv2.imdecode(dataFrame, cv2.IMREAD_COLOR)
    
    # 显示
    cv2.imshow('SERVER',decimg)
    if cv2.waitKey(1) == 27:
        break 
    idx += 1
    print('idx:', idx)

s.close()
cv2.destroyAllWindows()