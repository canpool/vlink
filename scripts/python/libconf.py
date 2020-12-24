# Copyright (c) [2020] vlink Team. All rights reserved.
# SPDX-License-Identifier: MulanPSL-2.0

import sys
import os
import time
import re
import pickle
import configparser
import tempfile
import shutil

# add to kconfig import path
sys.path.append(os.path.abspath(os.path.dirname(__file__) + "/../kconfig/"))

from kconfiglib import Kconfig

root_dir = os.path.abspath(os.path.dirname(__file__) + "/../..")

# kconfig handle
kconfig = None

project = None
prjname = os.getcwd().split(sep = "/")[-1] + ".prj"


def get_bsps():
    bsps = {}
    bsp_root = os.path.join(root_dir, "platform/board")
    for name in os.listdir(bsp_root):
        path = os.path.join(bsp_root, name)
        if os.path.isfile(path):
            continue
        if os.path.isfile(os.path.join(path, "kconfig")):
            bsps[name] = path
    return bsps


def __get_socs(dir, socs):
    for name in os.listdir(dir):
        path = os.path.join(dir, name)
        if os.path.isdir(path):
            __get_socs(path, socs)
            continue
        match = re.match("(.+)\.config", name)
        if not match:
            continue
        soc = match.groups()[0]
        if soc == "soc":
            continue
        if soc in socs:
            print("redefined SOC " + soc + " at:")
            print(path)
            print("and\n" + socs[soc])
            quit()
        socs[soc] = path


def get_socs(root):
    socs = {}
    __get_socs(os.path.join(root, "platform/soc"), socs)
    return socs


def get_soc(bsp):
    kconf = Kconfig(os.path.join(bsp, "kconfig"))
    if ("SOC" not in kconf.syms) or (not kconf.syms["SOC"].str_value):
        print("invalid bsp, SOC is not defined!")
        quit()
    return kconf.syms["SOC"].str_value


def get_project():
    global project

    if project:
        return project

    if os.path.exists(prjname):
        with open(prjname, "rb") as f:
            project = pickle.load(f)

    return project


def create_dir(path, force):
    if os.path.isfile(path):
        print("cannot create dir " + path + ", it is a file!")
        quit()

    if not os.path.isdir(path):
        os.makedirs(path)

    if not os.path.isdir(path):
        print("failed to create dir " + path)
        quit()

    if len(os.listdir(path)) != 0:
        if force:
            shutil.rmtree(path)
        else:
            print("dir " + path + " not empty!")
            quit()


def get_kconfig():
    global kconfig

    if kconfig:
        return kconfig

    project = get_project()

    if not project:
        return None

    os.environ.setdefault("KCONFIG_CONFIG", "config/kconfig.mk")

    kconfig = Kconfig("kconfig")

    kconfig.disable_redun_warnings()
    kconfig.disable_override_warnings()

    # load project kconfig.mk
    if os.path.isfile("config/kconfig.mk"):
        kconfig.load_config("config/kconfig.mk", False)
    else:
        soc_common = os.path.join(project["soc_dir"], "soc.config")
        if os.path.isfile(soc_common):
            kconfig.load_config(soc_common, False)

        # load soc.config
        soc_config = project["soc_cfg"]
        kconfig.load_config(soc_config, False)

        # load bsp.config
        bsp_config = os.path.join(project["bsp_dir"], "bsp.config")
        if os.path.isfile(bsp_config):
            kconfig.load_config(bsp_config, False)

    return kconfig


prj_kconfig = '''\
# created by project maintenance tool, do not edit it!

# add timestamp in the name to avoid potential name conflict
config PROJECT_{0}
    def_bool y
    select {1}

source "{2}/kconfig"

menu "platform (soc and board) configuration"

source "{3}/kconfig"
{4}
endmenu

'''

def _relcurpath(path):
    return os.path.relpath(path, os.path.curdir)

def parse_workspace(workspace, force):
    global project
    global prjname

    if workspace:
        proj, bsp = workspace

        prjname = proj

        bsps = get_bsps()

        if not bsp in bsps:
            print("bsp <" + bsp + "> not supported! Supported bsps are: " + str(list(bsps)))
            quit()

        bsp_dir = bsps[bsp]

        socs = get_socs(root_dir)

        soc  = get_soc(bsp_dir)

        if not soc in socs:
            print("soc <" + soc + "> not supported! Supported socs are: " + str(list(socs)))
            quit()

        soc_cfg = socs[soc]
        soc_dir = os.path.dirname(soc_cfg)

        create_dir(proj, force)
        create_dir(proj + "/config", force)

        project = {"root"     : root_dir,
                   "bsp"      : bsp,
                   "bsp_dir"  : bsp_dir,
                   "soc"      : soc,
                   "soc_dir"  : soc_dir,
                   "soc_cfg"  : soc_cfg,
                   "src"      : [],
                   "inc-l"    : [],
                   "inc-g"    : [],
                   "cflags-l" : [],
                   "cflags-g" : []}

        # now, goto the project dir
        os.chdir(proj)

        # copy main.c
        shutil.copyfile(os.path.join(root_dir, "link/main.c"), "main.c")
        project["src"].append("main.c")

        # create kconfig
        with open("kconfig", "w", encoding = "utf-8") as f:
            kconfig_list = []
            kconfig_dir  = soc_dir
            while kconfig_dir != root_dir:
                if os.path.isfile(os.path.join(kconfig_dir, "kconfig")):
                    kconfig_list.insert(0, os.path.join(kconfig_dir, "kconfig"))
                kconfig_dir = os.path.dirname(kconfig_dir)

            f.write(prj_kconfig.format(int(time.time()), bsp.upper(), _relcurpath(root_dir), _relcurpath(bsp_dir),
                                       "".join(["source \"" + _relcurpath(x) + "\"\n" for x in kconfig_list])))
        kconf = get_kconfig()

        # create Makefile
        with open("Makefile", "w", encoding = "utf-8") as f:
            f.write("# created by project maintenance tool, do not edit it!\n")
            f.write("root_dir   := $(abspath $(PWD)/%s)\n" % (_relcurpath(root_dir)))

            f.write("include $(root_dir)/scripts/make/entry.mk")
    else:
        project = get_project()

        if not project:
            print("Error: not in a project workspace!")
            quit()

    return project


def kconfig_set(content):
    if not content:
        return

    kconf = get_kconfig()

    tmphandle, tmppath = tempfile.mkstemp(prefix = "rtcfg.", text=True)
    tmpfile = os.fdopen(tmphandle, "w")

    for s in content:
        match = re.match("CONFIG_([^=]+)=(.*)", s)
        if not match:
            print(s + " not match the 'CONFIG_XX=x' format, assignment ignored.")
            continue
        name, val = match.groups()
        if name not in kconf.syms:
            print("kconfig symbol CONFIG_" + name + " not exist, assignment ignored.")
        else:
            tmpfile.write(s + "\n")

    tmpfile.close()
    warn_to_stderr = kconf.warn_to_stderr
    # do not put warnings to console directly, we will handle it (strip the temp file name)
    kconf.warn_to_stderr = False
    kconf.load_config(tmppath, False)
    kconf.warn_to_stderr = warn_to_stderr
    os.remove(tmppath)

    # handle warnings and ignore the temp file name in it
    for e in kconf.warnings:
        print(re.match(".*(warning:.*)", e).groups()[0])

    kconf.write_config("config/kconfig.mk")
    kconf.write_autoconf("config/kconfig.h")
