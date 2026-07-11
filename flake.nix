{
  description = "STM32F407 'rx' firmware — ARM embedded toolchain dev shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "disco1-control";

          packages = with pkgs; [
            # ARM cross toolchain: arm-none-eabi-{gcc,g++,objcopy,size,gdb}
            gcc-arm-embedded

            # Build system used by ccf.sh / CMakePresets.json
            cmake
            ninja
            gnumake

            # Flashing + on-target debugging (provides st-flash / st-util)
            stlink

            # Handy for editor tooling (compile_commands.json is already generated)
            clang-tools
          ];

          shellHook = ''
            echo "disco1-control dev shell"
            echo "  arm-none-eabi-gcc : $(arm-none-eabi-gcc --version | head -n1)"
            echo "  cmake            : $(cmake --version | head -n1)"
            echo "  st-flash         : $(st-flash --version 2>&1 | head -n1)"
            echo ""
            echo "Build+flash:  ./ccf.sh    (flash step uses run0/sudo for USB access)"
            echo "Preset build: cmake --preset Debug && cmake --build build/Debug"
          '';
        };
      });
}
