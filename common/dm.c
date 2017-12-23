/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <stddef.h>
#include <string.h>

extern struct device device_list[];
extern struct device device_list_end[];

static int
device_probe(struct device *dev)
{
	int err;

	assert(dev->drv);
	assert(dev->drv->probe);

	if (dev->flags & DEVICE_FLAG_RUNNING)
		return SUCCESS;
	if (dev->bus && (err = device_probe(dev->bus)))
		return err;
	if (dev->clockdev && (err = device_probe(dev->clockdev)))
		return err;
	if (dev->irqdev && (err = device_probe(dev->irqdev)))
		return err;
	if ((err = dev->drv->probe(dev)))
		return err;

	debug("finished probing device %s", dev->name);
	dev->flags |= DEVICE_FLAG_RUNNING;
	return SUCCESS;
}

struct device *
dm_get_by_name(const char *name)
{
	struct device *dev;

	for (dev = device_list; dev < device_list_end; ++dev)
		if (!strcmp(dev->name, name))
			return dev;

	return NULL;
}

void
dm_init(void)
{
	struct device *dev;
	int err;

	for (dev = device_list; dev < device_list_end; ++dev) {
		if ((err = device_probe(dev))) {
			if (err == ENODEV)
				warn("failed to probe missing device %s",
				     dev->name);
			else
				panic("failed to probe device %s (%d)",
				      dev->name, err);
		}
	}
}
