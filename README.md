## Fdmpd project

### Prerequisites
libcurl-dev  
libcurlpp-dev  

### Installation
make  
make install  

### Usage
Project installs two binary files:  
fdmpd - daemon, that connects to the kernel part of fdmp and 
controls it using range of heuristics.  
fdmp-ctl - command line tool to explicitly show/change fdmp 
kernel part configuration.  

#### fdmpd
Daemon runs set of heuristics at some period and reconfigures fdmp based on these heuristics.  
Also it informs SDN controller when paths should be routed.  

Usage: fdmpd config_path opts_path  

While starting fdmpd loads:  
1) Configuration file  
2) Options file  
using 'config_path' and 'opts_path' arguments.  

Configuration file contains information about initial fdmp state.  
Each line must have following template:  
remote_name local_ip remote_ip  
When daemon starts current fdmp configuration will be cleaned and replaced with loaded.  

Options file has '.ini' format, it contains set of key-value pairs.  
Some heuristics variables and global params can be configured in this file.

Currently 3 heuristics are running:  
1) Duration heuristic. Counts average duration for every remote and if its value is greater than
option "time_to_add", heuristic checks whether one of the available tunnels has 0 flows. If it 
is true, heuristic adds one flow to one of the tunnels with 0 flows;  
2) Loss heuristic. Finds for every remote tunnel with the worst loss value and if this value is 
greater than "loss_to_del" option, deletes that tunnel;  
3) RTT heuristic. Checks rtt and loss value of every tunnel, if rtt is greater than "rtt_to_add",
loss is lesser than "small_loss" option and flows number on this tunnel is lesser than "max_flows",
this heuristic adds one flow to this tunnel.    

#### fdmp-ctl
Commands:  
list-remotes  
add-remote remote_name  
del-remote remote_name  
clear-remotes  
list-tunnels remote_name  
add-tunnel remote_name local_ip remote_ip  
del-tunnel local_ip remote_ip  
clear-tunnels  
tunnel-stat local_ip remote_ip  