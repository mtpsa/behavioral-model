#!/usr/bin/env python2

import runtime_CLI

import sys
import os
import json

import bmpy_utils
from mtpswitch_runtime import MtPsaSwitch

class MtPsaSwitchAPI(runtime_CLI.RuntimeAPI):
    @staticmethod
    def get_thrift_services():
        return [("mtpsa_switch", MtPsaSwitch.Client)]

    def __init__(self, pre_type, standard_client, mc_client, pswitch_client):
        runtime_CLI.RuntimeAPI.__init__(self, pre_type, standard_client, mc_client)
        self.pswitch_client = pswitch_client

    def do_set_queue_depth(self, line):
        """Set depth of one / all egress queue(s): set_queue_depth <nb_pkts> [<egress_port>]"""
        args = line.split()
        depth = int(args[0])
        if len(args) > 1:
            port = int(args[1])
            self.pswitch_client.set_egress_queue_depth(port, depth)
        else:
            self.pswitch_client.set_all_egress_queue_depths(depth)

    def do_set_queue_rate(self, line):
        """Set rate of one / all egress queue(s): set_queue_rate <rate_pps> [<egress_port>]"""
        args = line.split()
        rate = int(args[0])
        if len(args) > 1:
            port = int(args[1])
            self.pswitch_client.set_egress_queue_rate(port, rate)
        else:
            self.pswitch_client.set_all_egress_queue_rates(rate)

    def do_mirroring_add(self, line):
        """Add mirroring mapping: mirroring_add <mirror_id> <egress_port>"""
        args = line.split()
        mirror_id, egress_port = int(args[0]), int(args[1])
        self.pswitch_client.mirroring_mapping_add(mirror_id, egress_port)

    def do_mirroring_delete(self, line):
        """Delete mirroring mapping: mirroring_delete <mirror_id>"""
        self.pswitch_client.mirroring_mapping_delete(int(line))

    def do_get_time_elapsed(self, line):
        """Get time elapsed (in microseconds) since the switch started: get_time_elapsed"""
        print(self.pswitch_client.get_time_elapsed_us())

    def do_get_time_since_epoch(self, line):
        """Get time elapsed (in microseconds) since the switch clock's epoch: get_time_since_epoch"""
        print(self.pswitch_client.get_time_since_epoch_us())

    def do_load_user_config(self, line):
        """Load user json config: load_user_config <UserID> <path to .json file>"""
        args = line.split()
        self.exactly_n_args(args, 2)
        user_id = int(args[0])
        filename = args[1]

        if not os.path.isfile(filename):
            print("Error: Invalid filename: " + filename)
            return

        if 1 > user_id or user_id > 16:
            print("Error: UserID must be between 1 and 16")
            return

        print("Loading json config")
        if self.pswitch_client.load_user_config(user_id, filename):
            print("Error: Failed to load user config")
            return

    def do_switch_context(self, line):
        """Load config from context: switch_context <ContextID>"""
        args = line.split()
        self.exactly_n_args(args, 1)
        self.ctx_id = int(args[0])
        runtime_CLI.load_json_str(
          bmpy_utils.get_json_config(
              standard_client=self.client,
              ctx_id=self.ctx_id
          )
        )
        return

def main():
    args = runtime_CLI.get_parser().parse_args()
    args.pre = runtime_CLI.PreType.SimplePreLAG
    services = runtime_CLI.RuntimeAPI.get_thrift_services(args.pre)
    services.extend(MtPsaSwitchAPI.get_thrift_services())

    standard_client, mc_client, pswitch_client = runtime_CLI.thrift_connect(args.thrift_ip, args.thrift_port, services)
    runtime_CLI.load_json_config(standard_client, args.json)
    MtPsaSwitchAPI(args.pre, standard_client, mc_client, pswitch_client).cmdloop()

if __name__ == '__main__':
    main()
