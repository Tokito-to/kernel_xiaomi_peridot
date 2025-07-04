load(":image_opts.bzl", "boot_image_opts")
load(":msm_kernel_le.bzl", "define_msm_le")
load(":target_variants.bzl", "le_64_variants")

target_name = "neo-le"

def define_neo_le():
    _neo_in_tree_modules = [
        # keep sorted
        "drivers/base/regmap/qti-regmap-debugfs.ko",
        "drivers/bluetooth/btpower.ko",
        "drivers/bus/mhi/devices/mhi_dev_dtr.ko",
        "drivers/bus/mhi/devices/mhi_dev_uci.ko",
        "drivers/bus/mhi/host/mhi.ko",
        "drivers/char/rdbg.ko",
        "drivers/clk/qcom/camcc-neo.ko",
        "drivers/clk/qcom/clk-dummy.ko",
        "drivers/clk/qcom/clk-qcom.ko",
        "drivers/clk/qcom/clk-rpmh.ko",
        "drivers/clk/qcom/clk-spmi-pmic-div.ko",
        "drivers/clk/qcom/debugcc-neo.ko",
        "drivers/clk/qcom/dispcc-neo.ko",
        "drivers/clk/qcom/gcc-neo.ko",
        "drivers/clk/qcom/gdsc-regulator.ko",
        "drivers/clk/qcom/gpucc-neo.ko",
        "drivers/clk/qcom/tcsrcc-neo.ko",
        "drivers/clk/qcom/videocc-neo.ko",
        "drivers/cpufreq/qcom-cpufreq-hw.ko",
        "drivers/cpufreq/qcom-cpufreq-hw-debug.ko",
        "drivers/cpuidle/governors/qcom_lpm.ko",
        "drivers/dma-buf/heaps/qcom_dma_heaps.ko",
        "drivers/dma/qcom/bam_dma.ko",
        "drivers/dma/qcom/msm_gpi.ko",
        "drivers/edac/kryo_arm64_edac.ko",
        "drivers/edac/qcom_edac.ko",
        "drivers/firmware/qcom-scm.ko",
        "drivers/hwspinlock/qcom_hwspinlock.ko",
        "drivers/hwtracing/coresight/coresight.ko",
        "drivers/hwtracing/coresight/coresight-csr.ko",
        "drivers/hwtracing/coresight/coresight-cti.ko",
        "drivers/hwtracing/coresight/coresight-dummy.ko",
        "drivers/hwtracing/coresight/coresight-funnel.ko",
        "drivers/hwtracing/coresight/coresight-remote-etm.ko",
        "drivers/hwtracing/coresight/coresight-replicator.ko",
        "drivers/hwtracing/coresight/coresight-stm.ko",
        "drivers/hwtracing/coresight/coresight-tgu.ko",
        "drivers/hwtracing/coresight/coresight-tmc.ko",
        "drivers/hwtracing/coresight/coresight-tmc-sec.ko",
        "drivers/hwtracing/coresight/coresight-tpda.ko",
        "drivers/hwtracing/coresight/coresight-tpdm.ko",
        "drivers/hwtracing/stm/stm_console.ko",
        "drivers/hwtracing/stm/stm_core.ko",
        "drivers/hwtracing/stm/stm_ftrace.ko",
        "drivers/hwtracing/stm/stm_p_basic.ko",
        "drivers/hwtracing/stm/stm_p_ost.ko",
        "drivers/hwtracing/stm/stm_p_sys-t.ko",
        "drivers/i2c/busses/i2c-msm-geni.ko",
        "drivers/iio/adc/qcom-spmi-adc5.ko",
        "drivers/iio/adc/qcom-vadc-common.ko",
        "drivers/input/misc/qpnp-power-on.ko",
        "drivers/interconnect/icc-test.ko",
        "drivers/interconnect/qcom/icc-bcm-voter.ko",
        "drivers/interconnect/qcom/icc-debug.ko",
        "drivers/interconnect/qcom/icc-rpmh.ko",
        "drivers/interconnect/qcom/qnoc-neo.ko",
        "drivers/interconnect/qcom/qnoc-qos.ko",
        "drivers/iommu/arm/arm-smmu/arm_smmu.ko",
        "drivers/iommu/iommu-logger.ko",
        "drivers/iommu/msm_dma_iommu_mapping.ko",
        "drivers/iommu/qcom_iommu_debug.ko",
        "drivers/iommu/qcom_iommu_util.ko",
        "drivers/irqchip/qcom-pdc.ko",
        "drivers/leds/leds-aw2016.ko",
        "drivers/mailbox/msm_qmp.ko",
        "drivers/mailbox/qcom-ipcc.ko",
        "drivers/mfd/qcom-i2c-pmic.ko",
        "drivers/mfd/qcom-spmi-pmic.ko",
        "drivers/misc/qseecom_proxy.ko",
        "drivers/mmc/host/cqhci.ko",
        "drivers/mmc/host/sdhci-msm.ko",
        "drivers/mmc/host/sdhci-msm-scaling.ko",
        "drivers/nvmem/nvmem_qcom-spmi-sdam.ko",
        "drivers/pci/controller/pci-msm-drv.ko",
        "drivers/perf/qcom_llcc_pmu.ko",
        "drivers/pinctrl/qcom/pinctrl-msm.ko",
        "drivers/pinctrl/qcom/pinctrl-neo.ko",
        "drivers/pinctrl/qcom/pinctrl-spmi-gpio.ko",
        "drivers/pinctrl/qcom/pinctrl-spmi-mpp.ko",
        "drivers/power/reset/qcom-dload-mode.ko",
        "drivers/power/reset/qcom-reboot-reason.ko",
        "drivers/power/supply/bq256xx_charger.ko",
        "drivers/power/supply/bq27xxx_battery.ko",
        "drivers/power/supply/bq27xxx_battery_i2c.ko",
        "drivers/regulator/qcom-rpmh-regulator.ko",
        "drivers/regulator/qcom_pm8008-regulator.ko",
        "drivers/regulator/rpmh-regulator.ko",
        "drivers/regulator/stub-regulator.ko",
        "drivers/remoteproc/qcom_pil_info.ko",
        "drivers/remoteproc/qcom_q6v5.ko",
        "drivers/remoteproc/qcom_q6v5_pas.ko",
        "drivers/remoteproc/qcom_sysmon.ko",
        "drivers/remoteproc/rproc_qcom_common.ko",
        "drivers/rpmsg/glink_pkt.ko",
        "drivers/rpmsg/qcom_glink.ko",
        "drivers/rpmsg/qcom_glink_smem.ko",
        "drivers/rpmsg/qcom_smd.ko",
        "drivers/rtc/rtc-pm8xxx.ko",
        "drivers/slimbus/slim-qcom-ngd-ctrl.ko",
        "drivers/slimbus/slimbus.ko",
        "drivers/soc/qcom/boot_stats.ko",
        "drivers/soc/qcom/cdsprm.ko",
        "drivers/soc/qcom/cmd-db.ko",
        "drivers/soc/qcom/core_hang_detect.ko",
        "drivers/soc/qcom/crypto-qti.ko",
        "drivers/soc/qcom/dcc_v2.ko",
        "drivers/soc/qcom/dcvs/bwmon.ko",
        "drivers/soc/qcom/dcvs/dcvs_fp.ko",
        "drivers/soc/qcom/dcvs/memlat.ko",
        "drivers/soc/qcom/dcvs/qcom-dcvs.ko",
        "drivers/soc/qcom/dcvs/qcom-pmu-lib.ko",
        "drivers/soc/qcom/debug_symbol.ko",
        "drivers/soc/qcom/eud.ko",
        "drivers/soc/qcom/glink_probe.ko",
        "drivers/soc/qcom/hwkm.ko",
        "drivers/soc/qcom/llcc-qcom.ko",
        "drivers/soc/qcom/llcc_perfmon.ko",
        "drivers/soc/qcom/mdt_loader.ko",
        "drivers/soc/qcom/mem-hooks.ko",
        "drivers/soc/qcom/mem_buf/mem_buf.ko",
        "drivers/soc/qcom/mem_buf/mem_buf_dev.ko",
        "drivers/soc/qcom/memory_dump_v2.ko",
        "drivers/soc/qcom/minidump.ko",
        "drivers/soc/qcom/msm_performance.ko",
        "drivers/soc/qcom/pdr_interface.ko",
        "drivers/soc/qcom/pmic_glink.ko",
        "drivers/soc/qcom/power_state.ko",
        "drivers/soc/qcom/qcom_aoss.ko",
        "drivers/soc/qcom/qcom_cpu_vendor_hooks.ko",
        "drivers/soc/qcom/qcom_cpuss_sleep_stats.ko",
        "drivers/soc/qcom/qcom_logbuf_vendor_hooks.ko",
        "drivers/soc/qcom/qcom_ramdump.ko",
        "drivers/soc/qcom/qcom_rpmh.ko",
        "drivers/soc/qcom/qcom_soc_wdt.ko",
        "drivers/soc/qcom/qcom_va_minidump.ko",
        "drivers/soc/qcom/qcom_wdt_core.ko",
        "drivers/soc/qcom/qmi_helpers.ko",
        "drivers/soc/qcom/smem.ko",
        "drivers/soc/qcom/smp2p.ko",
        "drivers/soc/qcom/smp2p_sleepstate.ko",
        "drivers/soc/qcom/soc_sleep_stats.ko",
        "drivers/soc/qcom/socinfo.ko",
        "drivers/soc/qcom/sps/sps_drv.ko",
        "drivers/soc/qcom/sys_pm_vx.ko",
        "drivers/soc/qcom/sysmon_subsystem_stats.ko",
        "drivers/soc/qcom/tmecom/tmecom-intf.ko",
        "drivers/spi/spi-msm-geni.ko",
        "drivers/spmi/spmi-pmic-arb.ko",
        "drivers/spmi/spmi-pmic-arb-debug.ko",
        "drivers/thermal/qcom/cpu_hotplug.ko",
        "drivers/thermal/qcom/ddr_cdev.ko",
        "drivers/thermal/qcom/qcom-spmi-temp-alarm.ko",
        "drivers/thermal/qcom/qti_cpufreq_cdev.ko",
        "drivers/thermal/qcom/qti_devfreq_cdev.ko",
        "drivers/thermal/qcom/qti_qmi_cdev.ko",
        "drivers/thermal/qcom/qti_userspace_cdev.ko",
        "drivers/thermal/qcom/thermal_pause.ko",
        "drivers/tty/serial/msm_geni_serial.ko",
        "drivers/usb/dwc3/dwc3-msm.ko",
        "drivers/usb/gadget/function/usb_f_diag.ko",
        "drivers/usb/gadget/function/usb_f_qdss.ko",
        "drivers/usb/phy/phy-generic.ko",
        "drivers/usb/phy/phy-msm-snps-eusb2.ko",
        "drivers/usb/phy/phy-msm-ssusb-qmp.ko",
        "drivers/usb/phy/phy-qcom-emu.ko",
        "drivers/usb/repeater/repeater.ko",
        "drivers/usb/repeater/repeater-i2c-eusb2.ko",
        "drivers/virt/gunyah/gh_msgq.ko",
        "drivers/virt/gunyah/gh_rm_drv.ko",
        "drivers/virt/gunyah/gh_virt_wdt.ko",
        "kernel/msm_sysstats.ko",
        "kernel/sched/walt/sched-walt.ko",
        "kernel/trace/qcom_ipc_logging.ko",
        "net/qrtr/qrtr.ko",
        "net/qrtr/qrtr-mhi.ko",
        "net/qrtr/qrtr-smd.ko",
    ]

    _neo_debug_in_tree_modules = _neo_in_tree_modules + [
        # keep sorted
    ]

    kernel_vendor_cmdline_extras = [
        # do not sort
        "console=ttyMSM0,115200n8",
        "qcom_geni_serial.con_enabled=1",
        "bootconfig",
    ]

    for variant in le_64_variants:
        if variant == "debug-defconfig":
            mod_list = _neo_debug_in_tree_modules
        else:
            mod_list = _neo_in_tree_modules

        define_msm_le(
            msm_target = target_name,
            variant = variant,
            defconfig = "build.config.msm.neo-le",
            in_tree_module_list = mod_list,
            target_variants = le_64_variants,
            boot_image_opts = boot_image_opts(
                boot_image_header_version = 4,
                base_address = 0x80000000,
                page_size = 4096,
            ),
        )
