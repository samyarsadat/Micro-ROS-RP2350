#  Raspberry Pi RP2350 MicroROS tests - The ROS Robot Project Research.
#  Copyright 2025 Samyar Sadat Akhavi
#  Written by Samyar Sadat Akhavi, 2025.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https: www.gnu.org/licenses/>.

FROM samyarsadat/custom_base_images:base-ros-jazzy


# Install pip
RUN apt-get update \
    && apt-get install -y python3-pip \
    && rm -rf /var/lib/apt/lists/* && apt-get autoremove && apt-get autoclean

# Pico setup
COPY install_pico.sh /pico_setup.sh
RUN export NONROOT_USERNAME=${USERNAME} \
    && bash /pico_setup.sh \
    && rm -rf /var/lib/apt/lists/* && apt-get autoremove && apt-get autoclean

# MicroROS agent helpers
USER ${USERNAME}
COPY uros_agent/* /home/$USERNAME/pico_ws/uros_agent/

# Copy configuration files
USER ${USERNAME}
COPY vscode_config/* /home/$USERNAME/pico_ws/.vscode/

# OpenOCD Debug Probe permission fix
USER root
RUN usermod -aG plugdev ${USERNAME}

# Raspberry Pi Pico TTY permission fix
USER root
RUN usermod -aG dialout ${USERNAME}
    
# Post create and start scripts
COPY post_create.sh /devcon_post_create.sh
COPY post_start.sh /devcon_post_start.sh
USER root
RUN chmod +x /devcon_post_start.sh && chmod +x /devcon_post_create.sh
USER ${USERNAME}