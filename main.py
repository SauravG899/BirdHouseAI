import firebase_admin
from firebase_admin import credentials, storage
import numpy as np
import cv2
import time
import tensorflow as tf
import pandas as pd


class image_arrays:
    def __init__(self, initial):
        self.initial = initial
    
    def get_initial(self):
        return self.initial

    def set_initial(self, new):
        self.initial = new

        
cred = credentials.Certificate("./key.json")

app = firebase_admin.initialize_app(cred,{'storageBucket' : 'birdhouseai-3373a.appspot.com'})

bucket = storage.bucket()
blob = bucket.get_blob("data/birdphoto_test.jpg")

arr = np.frombuffer(blob.download_as_string(), np.uint8)
firstinst = image_arrays(arr)

def access():
    temp = np.frombuffer(blob.download_as_string(), np.uint8)
    print(temp)
    print(firstinst.get_initial())

    if (np.array_equal(temp, firstinst.get_initial())):
        print("same")
        pass    
    else:
        print ('diff')
        firstinst.set_initial(temp)

        img = cv2.imdecode(temp,cv2.COLOR_BGR2BGR555)

        #cv2.imshow('image',img)
        #cv2.waitKey(0)

        temp = output(img)
        indexNumber = temp.index(temp.max())
        print(indexNumber)
        find_name(indexNumber)


def prepare_img(image):
    location = './birdphoto_test.jpg'
    imgsize = 224
    img_array = cv2.imread(location, cv2.IMREAD)
    new_array = cv2.resize(img_array, (imgsize, imgsize))
    return new_array.reshape(-1, imgsize, imgsize, 1)


def output(picture):
    model = tf.keras.model.load_model("ABC.h5")
    prediction = model.predict([prepare_img(picture)])
    return prediction[0]

def find_name(index): 
    data = pd.read_csv (r'C:\Users\General Use\Desktop\birdhouse\ml\100-birds-species.csv')   
    df = pd.DataFrame(data, columns= ['class_index','class'])
    name_dict = df.to_dict()
    
    for i in name_dict.keys:
        if i == index:
            return name_dict[i]

while True:
    access()
    time.sleep(5*60)



