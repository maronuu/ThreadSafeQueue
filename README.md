# ThreadSafeQueue

## What is this?
This is an interactive app of moving a point in a display, using a thread-safe queue.
The queue is implemented in the pthread library.

## Requirements
Tested on Ubuntu 20.04 LTS.
- Ubuntu 20.04 LTS (other distro probably also works)
- gcc
- pthread

## Install & Run

### 1. Clone this repository
```
cd ~
git clone https://github.com/maronuu/ThreadSafeQueue.git
```

### 2. Build
```
cd ~/ThreadSafeQueue
make
```

### 3. Run
```
cd ~/ThreadSafeQueue/build
./main
```

### 4. Enter the destination (y, x) of the point `@` until you're satisfied.
The point `@` will move to the destination.


![2021-06-07_215211](https://user-images.githubusercontent.com/63549742/121020160-12140800-c7db-11eb-9c8a-9ac11fc93e1b.png)

You don't have to wait until it stops!
Your input is to be pushed into a thread-safe queue, which can store and provide designated points to threads safely.

![2021-06-07_215320](https://user-images.githubusercontent.com/63549742/121020246-2821c880-c7db-11eb-9d6b-f4649ccf20de.png)


### 5. (Optional) Run `queueTest` to check if the queue is thread-safe.
```
cd ~/ThreadSafeQueue/build
./queueTest
```
The output will be
```
Start Testing...
Success!
```
