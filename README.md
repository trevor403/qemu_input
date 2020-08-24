# Description
In my workflow I have to interact with Virtual Machines that do not have the use QEMU emulated VGA. Therefore I can't use the built-in VNC server for Guest input. My workaround was to use a network enabled HMP prompt from secondary computer to provide PV input to the Guest machine.
This application takes advantage of the "grab" feature of a raw evdev device to dedicate a full mouse and keyboard to the guest. I also have a crude pthread implimentation for each device to recieve and process input, then write to the control socket.

# TODO
- ~Use a thread-safe queue for socket writes (currently relying on Linux kernel fd)~ There is nothing wrong with doing this.
- Switch to a userland input grabbing solution (possibly GTK or SDL with cursor grab)
- ~Write a QEMU hmp patch for lower length input strings (ex. "k x13" for key input)~ This is not viable. Also we don't need that kind of performance.
- Scancode translation instead of character lookups
- Switch from HMP to QMP
- Convert this into Golang
