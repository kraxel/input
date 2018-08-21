# -*-python-*-
project('input-tools', 'c')

mkname  = find_program('name.sh')
ev_hdr  = custom_target('ev',  output : 'EV.h',  command : [ mkname, 'EV'  ])
sw_hdr  = custom_target('sw',  output : 'SW.h',  command : [ mkname, 'SW'  ])
rel_hdr = custom_target('rel', output : 'REL.h', command : [ mkname, 'REL' ])
abs_hdr = custom_target('abs', output : 'ABS.h', command : [ mkname, 'ABS' ])
msc_hdr = custom_target('msc', output : 'MSC.h', command : [ mkname, 'MSC' ])
led_hdr = custom_target('led', output : 'LED.h', command : [ mkname, 'LED' ])
snd_hdr = custom_target('snd', output : 'SND.h', command : [ mkname, 'SND' ])
rep_hdr = custom_target('rep', output : 'REP.h', command : [ mkname, 'REP' ])
key_hdr = custom_target('key', output : 'KEY.h', command : [ mkname, 'KEY' ])
btn_hdr = custom_target('btn', output : 'BTN.h', command : [ mkname, 'BTN' ])
bus_hdr = custom_target('bus', output : 'BUS.h', command : [ mkname, 'BUS' ])
headers = [ ev_hdr, sw_hdr, rel_hdr, abs_hdr, msc_hdr, led_hdr,
            snd_hdr, rep_hdr, key_hdr, btn_hdr, bus_hdr ]

executable('lsinput',
           sources : [ 'lsinput.c', 'input.c', headers ],
           install : true)
install_man('lsinput.8')

executable('input-events',
           sources : [ 'input-events.c', 'input.c', headers ],
           install : true)
install_man('input-events.8')

executable('input-kbd',
           sources : [ 'input-kbd.c', 'input.c', headers ],
           install : true)
install_man('input-kbd.8')

executable('input-send',
           sources : [ 'input-send.c', 'input.c', 'tcp.c', headers ],
           install : true)

executable('input-recv',
           sources : [ 'input-recv.c', 'input.c', 'tcp.c', headers ],
           install : true)

executable('emulate-key',
           sources : [ 'emulate-key.c', 'input.c', headers ])
