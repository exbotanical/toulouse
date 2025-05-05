#ifndef DRIVER_DEV_CHAR_PCI_H
#define DRIVER_DEV_CHAR_PCI_H

typedef struct pci_device pci_device_t;

struct pci_device {
  unsigned char      bus;
  unsigned char      dev;
  unsigned char      func;
  unsigned short int vendor_id;
  unsigned short int device_id;
  unsigned short int command;
  unsigned short int status;
  unsigned char      rev;
  unsigned char      prog_if;
  unsigned short int class;
  unsigned char header;
  unsigned int  bar[6];
  unsigned char irq;
  unsigned char pin;
  unsigned int  size[6];
  pci_device_t *prev;
  pci_device_t *next;
};

#endif /* DRIVER_DEV_CHAR_PCI_H */
