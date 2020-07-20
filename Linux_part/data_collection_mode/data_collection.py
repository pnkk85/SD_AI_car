import cv2
from picamera.array import PiRGBArray
from picamera import PiCamera
import socket
import threading
import time
import os
import sys

sys.path.append('../myCNN')
import datasets


# create thread to load steer and throttle position to msg
def thread_function(name):
    global msg

    print("Thread %d started" % name)
    s = socket.socket()
    port = 8080
    s.connect(('127.0.0.1', port))
    while True:
        msg_lock.acquire()
        msg = s.recv(8)
        msg = msg.decode('utf-8').rstrip('\n')
        msg_lock.release()
        time.sleep(0.02)


if __name__ == "__main__":

    # dataset directory name
    DATASET_DIR = '/DATA/dataset_test_20200612_03'

    # camera settings
    IMG_WIDTH = 400
    IMG_HEIGHT = 300
    FRAME_RATE = 30

    # variables
    msg = ''
    image_name = ''

    # mutex
    msg_lock = threading.Lock()

    # initialize camera
    cap = PiCamera()
    cap.resolution = (IMG_WIDTH, IMG_HEIGHT)
    cap.framerate = FRAME_RATE
    rawCapture = PiRGBArray(cap, size=(IMG_WIDTH, IMG_HEIGHT))

    # allow the camera to warm-up
    time.sleep(0.1)

    # create directory for dataset
    try:
        os.mkdir(os.getcwd() + DATASET_DIR)
    except OSError:
        print("Creation of the directory %s failed" % os.getcwd() + DATASET_DIR)
    else:
        print("Successfully created the directory %s " % os.getcwd() + DATASET_DIR)

    # create daemon thread
    t1 = threading.Thread(target=thread_function, args=(1,), daemon=True)
    t1.start()

    save_counter = 0
    # capture frames from the camera
    for frame in cap.capture_continuous(rawCapture, format="bgr", use_video_port=True):

        # NumPy array representing the image
        image = frame.array

        # rotate frame
        image = cv2.rotate(image, cv2.ROTATE_180)

        # save 1/3 of images only
        if save_counter == 3:
            msg_lock.acquire()
            image_name = datasets.datetime_st_uuid4(msg)
            datasets.save_snapshot(os.getcwd() + DATASET_DIR, IMG_WIDTH, IMG_HEIGHT, image, image_name)
            msg_lock.release()
            save_counter = 0
        else:
            save_counter += 1

        # show the image
        #        cv2.imshow("Frame", image)

        # clear the stream for the next frame
        rawCapture.truncate(0)

        # break from the loop if 'q' pressed
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
