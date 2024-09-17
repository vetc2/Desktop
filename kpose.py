import cv2
import numpy as np
import mediapipe as mp

# Inisialisasi MediaPipe Pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose()

# Inisialisasi Kalman Filter
state = np.array([0, 0, 0, 0], dtype=float)
P = np.eye(4) * 1000
dt = 1
F = np.array([[1, 0, dt, 0],
              [0, 1, 0, dt],
              [0, 0, 1, 0],
              [0, 0, 0, 1]])
B = np.zeros((4, 2))
Q = np.eye(4) * 0.1
H = np.array([[1, 0, 0, 0],
              [0, 1, 0, 0]])
R = np.eye(2) * 0.5

# Fungsi untuk update Kalman Filter
def kalman_filter_update(state, P, z, F, B, u, H, R, Q):
    state_pred = F @ state + B @ u
    P_pred = F @ P @ F.T + Q

    y = z - H @ state_pred
    S = H @ P_pred @ H.T + R
    K = P_pred @ H.T @ np.linalg.inv(S)

    state_updated = state_pred + K @ y
    P_updated = (np.eye(len(P)) - K @ H) @ P_pred

    return state_updated, P_updated

# Fungsi untuk menghitung jarak antara dua titik
def calculate_distance(point1, point2):
    return np.sqrt((point1[0] - point2[0])**2 + (point1[1] - point2[1])**2)

# Buka video capture
cap = cv2.VideoCapture(0)

previous_nose_position = None
nose_id = 0

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        print("Tidak dapat membuka video.")
        break

    # Deteksi pose menggunakan MediaPipe
    image_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(image_rgb)

    if results.pose_landmarks:
        nose = results.pose_landmarks.landmark[mp_pose.PoseLandmark.NOSE]
        x_new, y_new = nose.x, nose.y

        # Update Kalman Filter
        z = np.array([x_new, y_new])
        u = np.zeros((2,))
        state, P = kalman_filter_update(state, P, z, F, B, u, H, R, Q)

        # Gunakan state yang diperbarui untuk stabilisasi pose
        stabilized_x, stabilized_y = state[0], state[1]

        # Periksa apakah hidung sama dengan perbandingan jarak
        if previous_nose_position is not None:
            distance = calculate_distance((stabilized_x, stabilized_y), previous_nose_position)
            if distance > 0.1:  # Threshold jarak untuk menentukan apakah titik hidung berbeda
                nose_id += 1
        previous_nose_position = (stabilized_x, stabilized_y)

        # Tampilkan hasil pada frame
        h, w, _ = frame.shape
        stabilized_x_pixel = int(stabilized_x * w)
        stabilized_y_pixel = int(stabilized_y * h)
        cv2.circle(frame, (stabilized_x_pixel, stabilized_y_pixel), 5, (0, 255, 0), -1)

        # Tampilkan ID hidung
        cv2.putText(frame, f"ID: {nose_id}", (stabilized_x_pixel + 10, stabilized_y_pixel - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 0), 2)

    # Tampilkan frame
    cv2.imshow('Stabilized Pose with Nose ID', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Bersihkan
cap.release()
cv2.destroyAllWindows()
