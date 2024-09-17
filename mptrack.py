import math
import cv2
import mediapipe as mp
from deep_sort_realtime.deepsort_tracker import DeepSort

# Inisialisasi MediaPipe Pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(min_detection_confidence=0.5, min_tracking_confidence=0.5)

# Inisialisasi DeepSORT
tracker = DeepSort()

def jarak(result, a, b, c):
    r = []
    r.append(result.landmark[a])
    r.append(result.landmark[b])
    d = math.sqrt((r[0].x - r[1].x)**2 + 
                  (r[0].y - r[1].y)**2 + 
                  (r[0].z - r[1].z)**2)
    return c/d

# Buka video atau webcam
cap = cv2.VideoCapture(0)
cap.set(3, 1366)
cap.set(4, 768)
cap.set(5, 30)

while cap.isOpened():
    ret, frame = cap.read()
    img = cv2.flip(frame, 1)
    
    # Ubah ke RGB untuk MediaPipe
    rgb_frame = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    
    # Deteksi pose dengan MediaPipe
    result = pose.process(rgb_frame).pose_landmarks
    
    detections = []
    if result:
        mp.solutions.drawing_utils.draw_landmarks(img, 
        result, mp_pose.POSE_CONNECTIONS)

        a = jarak(result, 9, 10, 0.033)
        b = jarak(result, 11, 12, 0.223)
        c = jarak(result, 23, 24, 0.133)
        print((a+b+c)/3.0, "\n")


        # Extract bounding box dari titik kunci MediaPipe
        landmarks = result.landmark
        x_min = min([lmk.x for lmk in landmarks])
        x_max = max([lmk.x for lmk in landmarks])
        y_min = min([lmk.y for lmk in landmarks])
        y_max = max([lmk.y for lmk in landmarks])
        
        bbox = [x_min * img.shape[1], 
                y_min * img.shape[0], 
                (x_max - x_min) * img.shape[1], 
                (y_max - y_min) * img.shape[0]]
        
        # conf = sum([1 for lmk in landmarks if lmk.visibility > 0.5]) / 33.0
        conf = 1.0
        classes = 0

        # DeepSORT expects [x_min, y_min, width, height], conf, classes
        detections.append((bbox, conf, classes))
    
    # Jalankan DeepSORT untuk melacak orang
    if len(detections) > 0:
        outputs = tracker.update_tracks(detections, frame=img)
        
        # Visualisasi hasil tracking
        for track in outputs:
            bbox = track.to_tlbr()
            
            # Gambar bounding box dan ID
            cv2.rectangle(img, (int(bbox[0]), int(bbox[1])), 
                          (int(bbox[2]), int(bbox[3])), (0, 255, 0), 2)
            
            # Posisi teks
            text_position = []
            text_position.append(10 if bbox[0] < 10 else int(bbox[0]))
            text_position.append(20 if bbox[1] < 30 else int(bbox[1] - 10))
            
            # Gambar teks ID
            cv2.putText(img, f'ID: {track.track_id}', text_position, 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    # Tampilkan frame
    cv2.imshow('Pose Tracking with DeepSORT', img)
    
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
