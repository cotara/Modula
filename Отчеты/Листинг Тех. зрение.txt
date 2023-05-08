
import cv2 
import numpy as np
import RPi.GPIO as GPIO

def cnt(lst):
    cS1=0
    cS2=0
    cS3=0
    font = cv2.FONT_HERSHEY_SIMPLEX 
    for (x, y, w, h) in lst:
        if(x+w/2 < frame_width/3):
            cS1=cS1+1
            cv2.rectangle(img,(x,y),(x+w,y+h),(0,255,0),2)
            cv2.putText(img, str(cS1), (x,y), font, 1, (0,0,255), 2)
        elif(x+w/2 > 2*frame_width/3):
            cS3=cS3+1
            cv2.rectangle(img,(x,y),(x+w,y+h),(0,0,255),2)
            cv2.putText(img, str(cS3), (x,y), font, 1, (0,0,255), 2)
        else:
            cS2=cS2+1
            cv2.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2)
            cv2.putText(img, str(cS2), (x,y), font, 1, (0,0,255), 2)
    return cS1,cS2,cS3

def faceDetect(img):
    face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
    gray  = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) 
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.5, minNeighbors=5) 
    
    return faces

def motionDetect():
    cont = []
    kernal = np.ones((5,5),np.uint8) 
    ret, img1 = cap.read()
    rayImage1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)  
    ret, img2 = cap.read() 
    rayImage2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY) 
    diffImage = cv2.absdiff(rayImage1,rayImage2) 
    blurImage = cv2.GaussianBlur(diffImage, (5,5), 0)
    _, thresholdImage = cv2.threshold(blurImage, 20,255,cv2.THRESH_BINARY)
    dilatedImage = cv2.dilate(thresholdImage,kernal,iterations=5)

    contours, _ = cv2.findContours (dilatedImage, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE) 
    
    for contour in contours: 
        if cv2.contourArea(contour) > motion_threshold:
            cont.append(cv2.boundingRect(contour))
    return cont

frame_width = 640
frame_height = 480
cap = cv2.VideoCapture(0) 
ret = cap.set(cv2.CAP_PROP_FRAME_WIDTH,frame_width)
ret = cap.set(cv2.CAP_PROP_FRAME_HEIGHT,frame_height)

GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.OUT)
GPIO.setup(22, GPIO.OUT)
GPIO.setup(23, GPIO.OUT)
GPIO.output(21, False)
GPIO.output(22, False)
GPIO.output(23, False)
cS1=0
cS2=0
cS3=0

factor=3
motion_threshold = 20000

while(True):
    ret, img = cap.read() 
    _faces = faceDetect(img)
    _cont = motionDetect()

    for face in _faces:
        _cont.append(face)

    cI1,cI2,cI3=cnt(_cont)
    
    #
    if(cI1>0):
        if(cS1<0):
            cS1=1
        else:
            cS1=cS1+1
    else:
        if(cS1>0):
            cS1=-1
        else:
            cS1=cS1-1
    #
    if(cI2>0):
        if(cS2<0):
            cS2=1
        else:
            cS2=cS2+1
    else:
        if(cS2>0):
            cS2=-1
        else:
            cS2=cS2-1
      #
    if(cI3>0):
        if(cS3<0):
            cS3=1
        else:
            cS3=cS3+1
    else:
        if(cS3>0):
            cS3=-1
        else:
            cS3=cS3-1
  
 
      
    if(cS1>=factor):
        GPIO.output(21, True)
    else:
        GPIO.output(21, False)
        
    if(cS2>=factor):
        GPIO.output(22, True)
    else:
        GPIO.output(22, False)
        
    if(cS3>=factor):
        GPIO.output(23, True)
    else:
        GPIO.output(23, False)
    
    cv2.imshow('Preview',img) 
    if cv2.waitKey(20) & 0xFF == ord('q'):
    	break

cap.release()
cv2.destroyAllWindows()

