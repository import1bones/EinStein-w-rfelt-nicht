import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers,Sequential


network=Sequential([layers.Dense(600,activation='relu'),
                   #layers.Dense(1200,activation='relu'),
                   #layers.Dense(1200,activation='relu'),
                   layers.Dense(600,activation='relu'),
                   layers.Dense(300,activation='relu'),
                   layers.Dense(6)])
network.build(input_shape=[None,150])
network.load_weights('.//weight/')

def GetValue(x):
    x=tf.reshape(x,[-1,150])
    out=network(x)
    out=tf.nn.softmax(out,axis=1)
    out=tf.argmax(out,axis=1)
    out=tf.cast(out,dtype=tf.int32)
    return int(out)

