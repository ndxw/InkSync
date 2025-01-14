import mouse
import keyboard
from time import sleep

deltas = []

def on_n():
    events = mouse.record(button='right', target_types=('down'))
    print(isinstance(events[0], mouse.ButtonEvent))

    positions = []
    is_pressed = []
    pressed = 0
    for event in events:
        if isinstance(event, mouse.ButtonEvent):
            if event.event_type == 'down':
                pressed = 1
            elif event.event_type == 'up':
                pressed = 0
        elif isinstance(event, mouse.MoveEvent):
            positions.append((event.x, event.y))
            is_pressed.append(pressed)
        else:
            continue

    deltas = []
    for i in range(len(positions)):
        try:
            delta_x = positions[i+1][0] - positions[i][0]
            delta_y = positions[i+1][1] - positions[i][1]
        except IndexError:
            delta_x = positions[0][0] - positions[i][0]
            delta_y = positions[0][1] - positions[i][1]
        deltas.append((delta_x, delta_y))

    print(f'{events[0]}, {events[1]}, {events[2]}')
    print(f'{positions[0]}, {positions[1]}, {positions[2]}')
    print(f'{deltas[0]}, {deltas[1]}')

    # write to file in format delta_x,delta_y,is_pressed
    f = open('mouse_deltas.txt', 'w')
    for i, delta in enumerate(deltas):
        f.write("{"+f"{deltas[i][0]},{deltas[i][1]},{int(is_pressed[i])}"+"},")
    f.close()
    exit()

keyboard.add_hotkey('n', on_n)
keyboard.wait()


