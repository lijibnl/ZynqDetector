GermaniumDetector Linux build (GNU Make)

Build:
	make

Build with simulation mode enabled:
	make SIM_MODE=1

Run:
	make run

Clean:
	make clean

Notes:

	- Requires pkg-config and libzmq development files. Install with:
    
      ```
      sudo apt update
      sudo apt install pkg-config
      sudo apt install libzmq3-dev
      ```

    - Generate build artifacts under `build/`.

	- The output binary is: `./GermaniumDetector`.
