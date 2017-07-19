
PRODUCT_PACKAGES := \
	 CinemaGraph \
	 ObjectRemover \
#    FMRadio
#    MyTube \
#    VideoPlayer


$(call inherit-product, $(SRC_TARGET_DIR)/product/common_wiko.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/telephony.mk)
# cinemagraph libs
PRODUCT_COPY_FILES += vendor/tinno/wiko/etc/cinemagraph_lib/libmorpho_cinema_graph.so:system/lib/libmorpho_cinema_graph.so
PRODUCT_COPY_FILES += vendor/tinno/wiko/etc/cinemagraph_lib/libmorpho_jpeg_io.so:system/lib/libmorpho_jpeg_io.so
PRODUCT_COPY_FILES += vendor/tinno/wiko/etc/cinemagraph_lib/libmorpho_memory_allocator.so:system/lib/libmorpho_memory_allocator.so
PRODUCT_COPY_FILES += vendor/tinno/wiko/etc/objecteraser_lib/libmorpho_object_remover_jni.so:system/lib/libmorpho_object_remover_jni.so
# Overrides
PRODUCT_BRAND  := WIKO
PRODUCT_MANUFACTURER := WIKO
PRODUCT_MODEL  := BIRDY
PRODUCT_NAME   := $(TARGET_PRODUCT)
PRODUCT_DEVICE := $(TARGET_PRODUCT)



# TINNO
PRODUCT_PACKAGE_OVERLAYS += vendor/tinno/wiko/overlay

#wiko_public
$(call inherit-product, vendor/tinno/wiko/wikoapk/wikoapk.mk)

