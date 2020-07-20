import tensorflow as tf
import numpy as np
from sklearn.model_selection import train_test_split
import datetime
import sys
import os
sys.path.append('../myCNN')
import models, datasets

DATA_DIRECTORY = 'DATA/dataset_home_twolines_1'
MODEL_DIRECTORY = 'saved_models'
MODEL_SHORT_DESCRIPTION = 'at_home_twolines'
IMG_WIDTH = datasets.get_image_width()
IMG_HEIGHT = datasets.get_image_height()

if __name__ == '__main__':

    # Configure GPU memory growth
    gpus = tf.config.experimental.list_physical_devices('GPU')
    if gpus:
        try:
            # Currently, memory growth needs to be the same across GPUs
            for gpu in gpus:
                tf.config.experimental.set_memory_growth(gpu, True)
            logical_gpus = tf.config.experimental.list_logical_devices('GPU')
            print(len(gpus), "Physical GPUs,", len(logical_gpus), "Logical GPUs")
        except RuntimeError as e:
            # Memory growth must be set before GPUs have been initialized
            print(e)

    # import data
    print('Importing dataset ...')
    directory = datasets.get_directory(DATA_DIRECTORY)
    image_paths = datasets.get_image_paths(directory)
    images = datasets.load_images_to_array(image_paths)

    # scale images to the range [0-1]
    images = images / 255.0
    images.max()

    # load steer and throttle dataset
    steer_throttle = datasets.load_steer_throttle_to_array(image_paths)

    # split to train and test sets
    (trainValX, testValX, trainImgX, testImgX) = train_test_split(steer_throttle, images, test_size=0.25, random_state=50)

    # scale steer an throttle to the range [0-1]
    max_steer = trainValX[:, 0].max()
    min_steer = trainValX[:, 0].min()
    max_throttle = trainValX[:, 1].max()
    min_throttle = trainValX[:, 1].min()
    trainY = [1 / (max_steer - min_steer) * (trainValX[:, 0] - min_steer),
              1 / (max_throttle - min_throttle) * (trainValX[:, 1] - min_throttle)]  # obtained from y = ax + b ...
    testY = [1 / (max_steer - min_steer) * (testValX[:, 0] - min_steer),
             1 / (max_throttle - min_throttle) * (testValX[:, 1] - min_throttle)]  # obtained from y = ax + b ...

    # transformation
    trainY = np.array(trainY).T
    testY = np.array(testY).T

    # the fun begins here!!!
    # create model
    model = models.create_cnn(IMG_WIDTH, IMG_HEIGHT)

    # compile model
    model.compile(loss='mean_squared_error', optimizer='adam', metrics=['accuracy'])

    # model summary
    print('Model summary:')
    model.summary()

    # train the model
    model.fit(trainImgX, trainY, validation_data=(testImgX, testY), epochs=4, batch_size=10)

    print('MAX steer: %d' % max_steer)
    print('MIN steer: %d' % min_steer)
    print('MAX throttle %d' % max_throttle)
    print('MIN throttle %d' % min_throttle)

    # get model name in proper format
    act_time = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    model_name_h5 = datasets.get_model_name(act_time, max_steer, min_steer, max_throttle, min_throttle, MODEL_SHORT_DESCRIPTION,'h5')

    # save h5 model
    model.save(os.path.join(MODEL_DIRECTORY, model_name_h5))

    # model quantization
    converter = tf.lite.TFLiteConverter.from_keras_model_file(os.path.join(MODEL_DIRECTORY, model_name_h5))
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    quant_model = converter.convert()

    # get model name in proper format
    model_name_tflite = datasets.get_model_name(act_time, max_steer, min_steer, max_throttle, min_throttle, MODEL_SHORT_DESCRIPTION, 'tflite')

    # save tflite model
    file = open(os.path.join(MODEL_DIRECTORY, model_name_tflite), 'wb')
    file.write(quant_model)
