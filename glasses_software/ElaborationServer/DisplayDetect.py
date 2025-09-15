import cv2
import queue
import threading

from traitlets import Any

class DisplayDetect(threading.Thread):
    def __init__(self, delay):
        # set up dello schermo
        super().__init__()
        self.daemon = True

        # creazione della queue di immagini
        self.image_queue = queue.Queue()
        self.delay = delay

    def run(self):
        # creo la finestra in cui mostrerò lo stream della camera
        cv2.namedWindow('ESP32 Camera', cv2.WINDOW_NORMAL)
        
        while True:
            try:
                # se ci sono frame da mostrare
                #  - li rimuovo dalla queue
                #  - li visualizzo e processo l'evento
                img = self.image_queue.get(timeout=1)
                cv2.imshow('ESP32 Camera', img)
                cv2.waitKey(self.delay)
                
            except queue.Empty:
                # se non ci sono frame da mostrare
                #  - processo l'evento
                #  - vado avanti
                cv2.waitKey(self.delay)
                continue    

    def add_frame(
            self, 
            img, 
            objects = None
        ) -> None:
        
        # copio da aggiungere nella queue
        edit = img.copy()

        if (objects is not None) and (len(objects) > 0):

            # se ci sono oggetti li disegno un punto sull'immagine in loro corrispondenza
            for obj in objects:
                cv2.circle(edit, (obj['center']), 5, (0, 0, 255), -1)

        # aggiungo l'immagine alla queue
        self.image_queue.put(edit)