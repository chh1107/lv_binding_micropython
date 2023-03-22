#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_gyroscope.h"

STATIC mp_obj_t init(void) {
	uint8_t data;
	data = BSP_GYRO_Init();
	return mp_obj_new_int(data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gyro_init_obj, init);

STATIC mp_obj_t read_id(void) {
	uint8_t data;
    data = BSP_GYRO_ReadID();

	return mp_obj_new_int(data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gyro_read_id_obj, read_id);

STATIC mp_obj_t read_xyz(void) {
	float data[3];
    BSP_GYRO_GetXYZ(data);

    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));
    tuple->items[0] = mp_obj_new_float(data[0]);
    tuple->items[1] = mp_obj_new_float(data[1]);
    tuple->items[2] = mp_obj_new_float(data[2]);
    return tuple;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gyro_read_xyz_obj, read_xyz);

STATIC const mp_rom_map_elem_t gyro_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_gyro) },
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&gyro_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read_id), MP_ROM_PTR(&gyro_read_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_xyz), MP_ROM_PTR(&gyro_read_xyz_obj) },
};
STATIC MP_DEFINE_CONST_DICT(gyro_module_globals, gyro_module_globals_table);

const mp_obj_module_t gyro_cmodule = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&gyro_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_stm32f429disc_gyro, gyro_cmodule);