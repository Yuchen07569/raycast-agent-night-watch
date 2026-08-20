# Contributing

1. Open an issue before changing power-management, authorization, recovery, or
   Windows Group Policy behavior.
2. Keep both platforms manual-only. Do not add schedules, agent monitoring,
   password storage, privilege bypasses, services, drivers, or DC/battery power
   mutations.
3. For macOS, run `npm test`, `npm run lint`, and `npm run build` from
   `raycast-extension/`.
4. For Windows, configure with CMake, build Release with MSVC, and run CTest.
   The core must continue to compile with a non-Windows C++20 compiler.
5. State the OS version, hardware, sleep model, power source, and physical lid
   test performed. Do not claim closed-lid reliability from a VM.
6. Do not submit generated binaries, dependencies, personal caches, journals,
   credentials, or secrets.
