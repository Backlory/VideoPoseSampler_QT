VideoPoseSampler alpha - demo version

A server that communicates simultaneously with NDI Aurora and OpenCV VideoCapture Device.

Used for capturing video streams and corresponding pose streams. Save only the original data without providing any interpolation or synchronization.

# Params：
-s saveFolderPath, where to save all the data

--roi x,y,w,h, the ROI of the image

--size H,W, frame size of device


# If saved locally：
A folder will be generated under the save folder (default is sampleoutput) to save frames in jpg format and poses in json format.

# If communicating with other processes in TCP mode：
Firstly, establish a TCP connection, and then call the getData function to retrieve the data.
