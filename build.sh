#!/usr/bin/env bash

set -euo pipefail

MODULE_NAME="procscope"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_DIR="${SCRIPT_DIR}/module"
MODULE_PATH="${MODULE_DIR}/${MODULE_NAME}.ko"

usage() {
	echo "Usage: $0 {-c|-i|-u|-C}"
	echo "  -c  build kernel module"
	echo "  -i  insert module; unload it first if already loaded"
	echo "  -u  unload module"
	echo "  -C  run make clean"
}

need_command() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "error: command not found: $1" >&2
		exit 1
	fi
}

run_privileged() {
	if [ "${EUID}" -eq 0 ]; then
		"$@"
		return
	fi

	need_command sudo
	sudo "$@"
}

module_state() {
	awk -v name="${MODULE_NAME}" '
		$1 == name {
			print $5
			found = 1
			exit
		}
		END {
			exit found ? 0 : 1
		}
	' /proc/modules
}

module_loaded() {
	module_state >/dev/null 2>&1
}

ensure_module_live() {
	local state

	state="$(module_state || true)"
	if [ -z "${state}" ]; then
		return 1
	fi

	if [ "${state}" != "Live" ]; then
		echo "error: ${MODULE_NAME} is in ${state} state, not Live" >&2
		echo "hint: the previous module load likely did not finish; a reboot may be required" >&2
		exit 1
	fi
}

wait_until_unloaded() {
	local i

	for ((i = 0; i < 50; i++)); do
		if ! module_loaded; then
			return
		fi
		sleep 0.1
	done

	echo "error: ${MODULE_NAME} is still present after rmmod" >&2
	exit 1
}

build_module() {
	need_command make
	make -C "${SCRIPT_DIR}"
}

clean_module() {
	need_command make
	make -C "${SCRIPT_DIR}" clean
}

insert_module() {
	need_command awk

	if [ ! -f "${MODULE_PATH}" ]; then
		echo "error: module file not found: ${MODULE_PATH}" >&2
		echo "hint: run ./build.sh -c first" >&2
		exit 1
	fi

	if module_loaded; then
		ensure_module_live
		echo "${MODULE_NAME} is already loaded, unloading first..."
		if ! run_privileged rmmod "${MODULE_NAME}"; then
			echo "error: failed to unload ${MODULE_NAME}" >&2
			exit 1
		fi
		wait_until_unloaded
	fi

	echo "inserting ${MODULE_PATH}..."
	if ! run_privileged insmod "${MODULE_PATH}"; then
		echo "error: failed to insert ${MODULE_PATH}" >&2
		exit 1
	fi
}

unload_module() {
	need_command awk

	if ! module_loaded; then
		echo "${MODULE_NAME} is not loaded"
		return
	fi

	ensure_module_live
	echo "unloading ${MODULE_NAME}..."
	if ! run_privileged rmmod "${MODULE_NAME}"; then
		echo "error: failed to unload ${MODULE_NAME}" >&2
		exit 1
	fi
	wait_until_unloaded
}

if [ "$#" -eq 0 ]; then
	usage
	exit 1
fi

while getopts ":ciuC" opt; do
	case "${opt}" in
	c)
		build_module
		;;
	i)
		insert_module
		;;
	u)
		unload_module
		;;
	C)
		clean_module
		;;
	?)
		usage
		exit 1
		;;
	esac
done
