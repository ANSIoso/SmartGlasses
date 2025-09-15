from flask import Flask, request

from ultralytics import YOLO
import cv2
import numpy as np

from DisplayDetect import DisplayDetect

# Global queue for images
display_thread = None
model = None

def detect(model, image):
    
    # effettuo la predizione per l'immagine
    results = model(image) 

    # estraggo dalla predizione per ogni oggetto
    # - una lista con informazioni sulla loro posizione
    # - una lista con la classe di tali oggetti
    xywh = None
    names = None
    for result in results:
        xywh = result.boxes.xywh 
        names = [result.names[cls.item()] for cls in result.boxes.cls.int()]  

    # dalla lista con le info sulle posizioni estraggo X e Y dei loro centri
    X = [int(position.item()) for position in xywh[:, 0]]
    Y = [int(position.item()) for position in xywh[:, 1]]
    centers = list(zip(X, Y))

    # creo una lista di riepilogo con tutti gli oggetti trovati
    objects = []
    for i, object_name in enumerate(names):
        # creo una struttura con le informazioni di un singolo oggetto
        obj = {}
        obj['class'] = object_name
        obj['center'] = centers[i]

        # aggiungo il nuovo oggetto alla lista di quelli trovati
        objects.append(obj)

    return objects

def extract_img(data):
    # Extract image from data
    # 1. Convert bytes to numpy array
    # 2. Decode image
    # 3. Ruoto l'immagine
    nparr = np.frombuffer(data, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    img = cv2.rotate(img, cv2.ROTATE_180)
    return img

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload_image():
    try:
        # Get data from the request
        image_data = request.get_data()
        if not image_data:
            return "No image data received", 400
        
        # trasformo i dati nell'immagine finale
        img = extract_img(image_data)
        if img is None:
            return "Failed to decode image", 400        

        print("richiesta")
        # predico gli oggetti nell'immagine
        objects = detect(model, img)

        print("elaborata")
        # print(img.shape)
        # print(objects)

        # visualizzo le predizioni per debug
        display_thread.add_frame(img, objects)
        
        return objects, 200
        
    except Exception as e:
        print(f"Error processing image: {str(e)}")
        return f"Error: {str(e)}", 500

@app.route('/status', methods=['GET'])
def status():
    return "Image receiver is running!", 200

if __name__ == '__main__':
    model = YOLO("yolo11m.pt")  # load an official model

    for name in model.model.names.values():
        print(name)

    display_thread = DisplayDetect(30)
    display_thread.start()

    # Run on all interfaces so ESP32 can reach it
    app.run(host='0.0.0.0', port=5000, debug=False)
