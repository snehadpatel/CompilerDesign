status1 = True
status2 = False

if status1 and not status2:
    print("Logic evaluates to True")

check = 15

if check > 10 and check < 20:
    print("Safe zone")
elif check <= 10 or check >= 20:
    print("Danger zone")
else:
    print("Unknown")
