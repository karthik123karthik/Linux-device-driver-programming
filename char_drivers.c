// Device Driver is a piece of code that configure and manages a device
// In Kernel space we can see different types of device drivers
// CHAR drivers, Block device drivers and network device drivers
// Kernel will interact with these drivers to perform I/O operations

// CHAR drivers : access data from the device sequentially that is byte by byte
// no advanced buffering startegy is required as data is going byte by byte
// eg : keyboard

//Block drivers : access data from the device in fixed-size blocks
// They are responsible for managing the read and write requests for these blocks
// Block drivers typically use a buffer cache to improve performance eg ssd

// Device numbers : kernel assigns one number to identify the device number containg 2 parts
// major number : minor number (actual device driver identifier and device instance)
