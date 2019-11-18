/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "vlink.h"

#ifdef CONFIG_OS
#include "vos.h"
#endif

#if CONFIG_NET
#include "sal.h"
#include "sal_linux.h"
#endif

#ifdef WITH_DTLS
#include "dtls.h"
#endif

int vlink_init(void)
{
#if CONFIG_OS
    vos_init();
#endif

#if CONFIG_NET
    sal_init(8);
#if CONFIG_NET_LINUX
    sal_install_linux();
#else
#error "please config net type"
#endif
#endif

    return 0;
}

int vlink_main(void *args)
{
#ifdef WITH_DTLS
    dtls_init();
#endif

/* coap */
#if CONFIG_COAP
    extern int coap_setup(void);
    coap_setup();
#endif // CONFIG_COAP

#if CONFIG_OC_COAP
    extern int oc_coap_setup(void);
    oc_coap_setup();
#endif

/* mqtt */
#if CONFIG_MQTT
    extern int mqtt_setup(void);
    mqtt_setup();
#endif

#if CONFIG_OC_MQTT
    extern int oc_mqtt_setup(void);
    oc_mqtt_setup();
#endif

#if CONFIG_DEMOS
    extern int standard_app_demo_main();
    standard_app_demo_main();
#endif

    return 0;
}