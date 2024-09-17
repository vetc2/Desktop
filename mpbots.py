import cv2
import mediapipe as mp
import numpy as np
from ultralytics.trackers.bot_sort import BOTrack

# Inisialisasi MediaPipe Pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose()


# Inisialisasi kamera
cap = cv2.VideoCapture(0)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    
    # Convert frame ke RGB (dibutuhkan oleh MediaPipe)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    # Proses frame untuk mendeteksi pose dengan MediaPipe
    results = pose.process(rgb_frame)
    
    if results.pose_landmarks:
        h, w, _ = frame.shape
        
        # Extract landmarks (titik tubuh) dari hasil MediaPipe Pose
        landmarks = []
        for lm in results.pose_landmarks.landmark:
            x = int(lm.x * w)
            y = int(lm.y * h)
            landmarks.append([x, y])
        
        # Convert landmarks menjadi bounding box (deteksi)
        x_min = min([lm[0] for lm in landmarks])
        x_max = max([lm[0] for lm in landmarks])
        y_min = min([lm[1] for lm in landmarks])
        y_max = max([lm[1] for lm in landmarks])
        
        # Format bounding box sebagai [x_min, y_min, x_max, y_max]
        bbox = [x_min, y_min, x_max, y_max]
        
        
        # Inisialisasi track untuk frame pertama atau jika diperlukan
        bo_track = BOTrack(bbox, 1.0, 0)
        print(bo_track)
        
        # Prediksi multi-tracks
        # tracks = bot_sort.multi_predict(tracks)
        
        # Gambar bounding box dan track ID dari tracker pada frame
        # for track in tracks:
        #     track_id = track.track_id  # Track ID dari BoT-SORT
        #     bbox = track
            # print(bbox)  # Bounding box [x_min, y_min, x_max, y_max]
            
            # Gambar bounding box
            # cv2.rectangle(frame, 
            #               (int(bbox[0]), int(bbox[1])), 
            #               (int(bbox[2]), int(bbox[3])),
            #               (0, 255, 0), 2)
            
            # # Tampilkan track ID di dekat bounding box
            # cv2.putText(frame, f'ID: {track_id}', (int(bbox[0]), int(bbox[1] - 10)), 
            #             cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 0, 0), 2)
    
    # Tampilkan frame
    cv2.imshow('BoT-SORT Pose Tracking', frame)
    
    # Tekan 'q' untuk keluar
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

# Bersihkan
cap.release()
cv2.destroyAllWindows()
