import threading

counter = 0
ac = []

def inc():
    global counter
    for i in range(10):
        counter += 1
        ac.append([(i+1, counter)])

threads = [threading.Thread(target=inc) for _ in range(2)]

for t in threads:
    t.start()

for t in threads:
    t.join()

print("Counter: ", ac)
