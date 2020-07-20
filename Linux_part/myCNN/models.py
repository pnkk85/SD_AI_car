import tensorflow as tf
from keras.models import Sequential
from keras.layers import Conv2D
from keras.layers import MaxPooling2D
from keras.layers import Flatten
from keras.layers import Dense
from keras.layers import Activation
import os


# creates cnn
def create_cnn(img_width, img_height):
    # Initialize CNN
    model = Sequential()

    # convolution layer
    model.add(Conv2D(filters=16, kernel_size=(3, 3), input_shape=(img_height, img_width, 1), activation='relu'))

    # pooling layer
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # convolution layer
    model.add(Conv2D(filters=32, kernel_size=(3, 3), input_shape=(img_height, img_width, 1), activation='relu'))

    # pooling layer
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # pooling layer
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # Flattening
    model.add(Flatten())

    # fully connected layers
    model.add(Dense(256))
    model.add(Activation('relu'))

    model.add(Dense(128))
    model.add(Activation('relu'))

    model.add(Dense(128))
    model.add(Activation('relu'))

    model.add(Dense(64))
    model.add(Activation('relu'))

    model.add(Dense(10))
    model.add(Activation('relu'))

    # output layer with linear activation function
    model.add(Dense(2))
    model.add(Activation('linear'))

    return model


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
