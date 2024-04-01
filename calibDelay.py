'''
Author: backlory's desktop dbdx_liyaning@126.com
Date: 2024-01-09 10:42:00
LastEditors: backlory's desktop dbdx_liyaning@126.com
LastEditTime: 2024-03-26 08:46:07
Description:

标定系统的采集延迟系数。

Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
'''
import cv2
import numpy as np
import open3d as o3d

from utils.dataset import USDatasetWithPosev3io


def _getImgMarkIdx(imageList: list) -> int:
    '''
    读取图像流，标记图像突变点。
    '''
    imgMarkIdx = -1
    cv2.namedWindow('Select imgMark, press Esc to confirm', cv2.WINDOW_NORMAL)
    cv2.resizeWindow('Select imgMark, press Esc to confirm', 600, 600)
    cv2.createTrackbar('idx', 'Select imgMark, press Esc to confirm', 0,
                       len(imageList) - 1, lambda x: None)
    while True:
        # 获取进度条值
        idx = cv2.getTrackbarPos('idx', 'Select imgMark, press Esc to confirm')
        # 显示图像
        cv2.imshow('Select imgMark, press Esc to confirm', imageList[idx])
        # 按下Esc键，退出循环
        if cv2.waitKey(1) == 27:
            imgMarkIdx = idx
            break
    cv2.destroyAllWindows()
    return imgMarkIdx


def _getPoseMarkIdx(poseList: list) -> int:
    '''
    读取位姿流，标记位姿突变点。
    '''
    poseMarkIdx = -1

    # 键盘回调事件
    def key_callback_left(vis):
        nonlocal poseMarkIdx, pcd
        pcd.colors[poseMarkIdx] = [255, 0, 0]
        poseMarkIdx = max(poseMarkIdx - 1, 0)
        pcd.colors[poseMarkIdx] = [0, 255, 255]
        # 刷新界面
        vis.update_geometry(pcd)
        return

    def key_callback_right(vis):
        nonlocal poseMarkIdx, pcd
        pcd.colors[poseMarkIdx] = [255, 0, 0]
        poseMarkIdx = min(poseMarkIdx + 1, len(points) - 1)
        pcd.colors[poseMarkIdx] = [0, 255, 255]
        vis.update_geometry(pcd)
        return

    # 创建点云
    points = []
    for pose in poseList:
        points.append(pose[:3, 3])
    points = o3d.utility.Vector3dVector(points)
    pcd = o3d.geometry.PointCloud()
    pcd.points = points
    pcd.colors = o3d.utility.Vector3dVector([[255, 0, 0]
                                             for i in range(len(points))])

    # 在相邻点之间创建线段
    lines = []
    for i in range(len(points) - 1):
        lines.append([i, i + 1])
    line_set = o3d.geometry.LineSet()
    line_set.points = points
    line_set.lines = o3d.utility.Vector2iVector(lines)
    line_set.colors = o3d.utility.Vector3dVector([[0, 0, 255]
                                                  for i in range(len(lines))])

    # 创建可视化窗口，增加进度条
    vis = o3d.visualization.VisualizerWithKeyCallback()
    vis.create_window(
        window_name=
        'Select poseMark, press left/right to select point, Esc to confirm')
    vis.get_render_option().background_color = np.asarray([0, 0, 0])
    vis.add_geometry(pcd)
    vis.add_geometry(line_set)
    # 键盘左右键
    vis.register_key_callback(262, key_callback_right)  # 按下右键
    vis.register_key_callback(263, key_callback_left)  # 按下左键
    vis.run()
    return poseMarkIdx


def delay_Calib(datasetDir=''):
    '''
    图像-位姿延迟标定。
    原理：
    在某一时间点，对绑定Aurora电磁传感器的成像设备施加外力，使得位姿、图像巨变，
    记录下此时的图像编号imgMarkIdx、位姿编号poseMardIdx，即可计算出图像-位姿延迟。
    '''
    print('读取数据集...', end='')
    dataset = USDatasetWithPosev3io(datasetDir=datasetDir)
    imageList = []
    poseList = []
    for i in range(len(dataset)):
        img, pose = dataset[i]
        imageList.append(img)
        poseList.append(pose)
    print('\033[1;33mdone!\033[0m')
    print(f'数据集中共有{len(dataset)}帧图像，{len(dataset)}帧位姿。')

    imgMarkIdx = _getImgMarkIdx(imageList)
    poseMardIdx = _getPoseMarkIdx(poseList)

    # 计算图像-位姿延迟
    delay = min(imgMarkIdx, poseMardIdx)
    temp = [imgMarkIdx - delay, poseMardIdx]
    print(f'使用标定延迟{temp}更新数据集...', end='')
    dataset.updateJson(frame_pose_startIdx=temp)
    print('\033[1;33mdone!\033[0m')
    return


if __name__ == '__main__':
    datasetDir = r'F:\Dataset\US\US_dataset_with_pose_v3'
    delay_Calib(datasetDir=datasetDir)
