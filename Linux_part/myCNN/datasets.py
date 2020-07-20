import glob
import os
import cv2
import numpy as np
import datetime
from uuid import uuid4


# ------------------------------------------------------------------
#
# Functions used for saving images with proper names
# Image names contain actual steer and throttle position
#
# ------------------------------------------------------------------

# returns image name in format
# <date>_<time>_<steer_throttle_str>_<uuid4>
def datetime_st_uuid4(steer_throttle_str):
    act_time = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    return '%s_%s_%s' % (act_time, steer_throttle_str, uuid4())


# save image
def save_snapshot(dataset_dir, width, height, image, name):
    image_path = os.path.join(dataset_dir, name + '.jpg')
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    image = cv2.resize(image, (width, height))
    cv2.imwrite(image_path, image)


# -----------------------------------------------------------------
#
# Functions used for getting information from image names and
# preparing data for cnn training
#
# -----------------------------------------------------------------

def get_image_width():
    return int(200)


def get_image_height():
    return int(150)


# returns path to directory
def get_directory(directory):
    return os.path.join(os.getcwd(), directory)


# returns list of image paths in given directory
def get_image_paths(directory):
    return glob.glob(os.path.join(directory, '*.jpg'))


# returns steer position saved in image name (image_path)
def get_steer_from_image_name(image_path):
    return float(os.path.basename(image_path)[16:20])


# returns throttle position saved in image name (image_path)
def get_throttle_from_image_name(image_path):
    return float(os.path.basename(image_path)[20:24])


# returns array of images given in image_paths
def load_images_to_array(image_paths):
    # initialize images list
    images = []

    for item in image_paths:
        # image scaling
        img = cv2.imread(item)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        img = cv2.resize(img, (200, 150))
        img = np.expand_dims(img, axis=2)
        images.append(img)

    # return images array
    return np.array(images)


# returns array of steer and throttle positions stored in image names
def load_steer_throttle_to_array(image_paths):
    steer_throttle = []

    # loop over image paths
    for item in image_paths:
        steer = get_steer_from_image_name(item)
        throttle = get_throttle_from_image_name(item)
        steer_throttle.append([steer, throttle])

    # return st array
    return np.array(steer_throttle)

# -----------------------------------------------------------------
#
# Functions used for getting information stored in model name
#
#
# -----------------------------------------------------------------

# returns model name in format:
# <date>_<time>_smaxx<max_steer>_smin<min_steer>_tmax<max_steer>_tmin<min_steer>_<own_comments_str>.<filetype>
def get_model_name(act_time, max_steer, min_steer, max_throttle, min_throttle, comments, filetype):
    return act_time + '_smax' + str(int(max_steer)) + '_smin' + str(int(min_steer)) + '_tmax' + str(
        int(max_throttle)) + '_tmin' + str(int(min_throttle)) + '_' + comments + '.' + filetype


# returns max steer value saved in model name (if model name obtained from get_model_name function)
def get_max_steer_from_model_name(model_name):
    return int(os.path.basename(model_name)[20:24])


# returns min steer value saved in model name (if model name obtained from get_model_name function)
def get_min_steer_from_model_name(model_name):
    return int(os.path.basename(model_name)[29:33])


# returns max throttle value saved in model name (if model name obtained from get_model_name function)
def get_max_throttle_from_model_name(model_name):
    return int(os.path.basename(model_name)[38:42])


# returns min throttle value saved in model name (if model name obtained from get_model_name function)
def get_min_throttle_from_model_name(model_name):
    return int(os.path.basename(model_name)[47:51])



# -----------------------------------------------------------------
#
# Functions used for cleaning up dataset from bad data
#
#
# -----------------------------------------------------------------

# removes images from dataset (image_paths) with throttle in range from throttle_min to throttle_max
# USE CAREFULLY!!!
def remove_bad_throttle_data(image_paths, throttle_min, throttle_max):
    counter = 0

    for item in image_paths:
        throttle = get_throttle_from_image_name(item)
        if throttle_min < throttle < throttle_max:
            counter += 1
            os.remove(item)
    print("%d bad data files removed from dataset" % counter)


# removes images from dataset (image_paths) with steer in range from steer_min to steer_max
# USE CAREFULLY!!!
def remove_not_steer_data(image_paths, steer_min, steer_max):
    counter = 0
    for item in image_paths:
        steer = get_steer_from_image_name(item)
        if steer_min < steer < steer_max:
            counter += 1
            os.remove(item)
    print("%d bad files removed from breaking data" % counter)


# SOME EXAMPLES:
if __name__ == "__main__":
    DATA_DIRECTORY = 'DATA/dataset_home_twolines_breaking/'

    print("dataset directory:")
    directory = get_directory(DATA_DIRECTORY)
    print(directory)

    print("image paths info:")
    image_paths = get_image_paths(directory)
    print(type(image_paths))
    print(directory)

    print("images_arr shape:")
    images_arr = load_images_to_array(image_paths)
    images_arr.shape

    print("st arr info:")
    st_arr = load_steer_throttle_to_array(image_paths)
    st_arr.shape

    print("value test:")
    print(image_paths[400])
    print(st_arr[400])

    print(len(image_paths))

    #    remove_bad_data(image_paths, 2360, 2480)
#    remove_not_breaking_data(image_paths, 2500)
#
#    image_paths = get_image_paths(directory)
#    print(len(image_paths))
