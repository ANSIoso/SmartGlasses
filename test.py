from flask import Flask, request
import cv2
import numpy as np
from datetime import datetime
import os

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload_image():
    try:
        # Get the image data from the request
        image_data = request.get_data()
        
        if not image_data:
            return "No image data received", 400
        
        # Convert bytes to numpy array
        nparr = np.frombuffer(image_data, np.uint8)
        
        # Decode image
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        
        if img is None:
            return "Failed to decode image", 400
        
        print(f"Received image: {img.shape}")
        
        # Save the image (optional)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"received_image_{timestamp}.jpg"
        cv2.imwrite(filename, img)
        print(f"Image saved as {filename}")
        
        # ===== YOUR IMAGE PROCESSING HERE =====
        # Example processing:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray, 100, 200)
        
        # You can add any processing you need:
        # - Object detection
        # - Face recognition  
        # - OCR
        # - Machine learning inference
        # - etc.
        
        # Example: Find contours
        contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        print(f"Found {len(contours)} contours")
        
        # Draw contours on original image
        result_img = img.copy()
        cv2.drawContours(result_img, contours, -1, (0, 255, 0), 2)
        
        # Save processed image
        processed_filename = f"processed_{timestamp}.jpg"
        cv2.imwrite(processed_filename, result_img)
        
        # ===== END PROCESSING =====
        
        return f"Image processed successfully. Found {len(contours)} objects.", 200
        
    except Exception as e:
        print(f"Error processing image: {str(e)}")
        return f"Error: {str(e)}", 500

@app.route('/status', methods=['GET'])
def status():
    return "Image receiver is running!", 200

if __name__ == '__main__':
    # Create directory for images if it doesn't exist
    os.makedirs('images', exist_ok=True)
    
    print("Starting image receiver server...")
    print("ESP32 should send images to: http://YOUR_PC_IP:5000/upload")
    
    # Run on all interfaces so ESP32 can reach it
    app.run(host='0.0.0.0', port=5000, debug=True)