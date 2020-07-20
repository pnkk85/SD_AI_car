from picamera.array import PiRGBArray
from picamera import PiCamera
import tflite_runtime.interpreter as tflite
import cv2
import time
import numpy as np
import socket
import os
import sys

sys.path.append('../myCNN')
import datasets

# model directory and name
MODEL_DIRECTORY = os.getcwd() + '/saved_models'
MODEL_NAME = '20200612_093138_smax3034_smin1620_tmax2427_tmin2295_at_home_twolines.tflite'

# variables for FPS calculation
fps = 1
freq = cv2.getTickFrequency()

# configure tcp client
s = socket.socket()
port = 8080
s.connect(('127.0.0.1', port))

# initialize the camera
IMG_WIDTH = datasets.get_image_width()
IMG_HEIGTH = datasets.get_image_height()
camera = PiCamera()
camera.resolution = (IMG_WIDTH, IMG_HEIGTH)
camera.framerate = 30
rawCapture = PiRGBArray(camera, size=(IMG_WIDTH, IMG_HEIGTH))

# camera warm-up
time.sleep(0.1)

# load model
model = tflite.Interpreter(model_path=os.path.join(MODEL_DIRECTORY, MODEL_NAME))

# get model details
input_details = model.get_input_details()
output_details = model.get_output_details()

# get values from model name to rescale cnn output
steer_max = datasets.get_max_steer_from_model_name(MODEL_NAME)
steer_min = datasets.get_min_steer_from_model_name(MODEL_NAME)
throttle_max = datasets.get_max_throttle_from_model_name(MODEL_NAME)
throttle_min = datasets.get_min_throttle_from_model_name(MODEL_NAME)


# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):

    # timer for FPS calculation
    t1 = cv2.getTickCount()

    # NumPy array representing image
    image = frame.array

    # show the frame
    image = cv2.rotate(image, cv2.ROTATE_180)
    #    cv2.imshow("Frame", image)

    # image preprocessing
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    image = cv2.resize(image, (IMG_WIDTH, IMG_HEIGTH))

    image = np.float32(image / 255)
    input_data = np.expand_dims(image, axis=0)
    input_data = np.expand_dims(input_data, axis=3)

    # prediction
    model.allocate_tensors()
    model.set_tensor(input_details[0]['index'], input_data)
    model.invoke()
    prediction = model.get_tensor(output_details[0]['index'])[0]

    # rescale cnn output
    steer_pred = int((steer_max - steer_min) * prediction[0] + steer_min)
    throttle_pred = int((throttle_max - throttle_min) * prediction[1] + throttle_min)

    # send msg over tcp socket
    msg = 's' + str(steer_pred) + 't' + str(throttle_pred)
    s.send(bytes(str(msg), 'utf8'))

    # clear the stream
    rawCapture.truncate(0)

    # FPS calculation
    t2 = cv2.getTickCount()
    duration = (t2 - t1) / freq
    fps = 1 / duration
    print('\r FPS: %.2f, s: %d, t: %d' % (fps, steer_pred, throttle_pred), end='')

    # press 'q' to break
#    if cv2.waitKey(1) & 0xFF == ord("q"):
#        break
