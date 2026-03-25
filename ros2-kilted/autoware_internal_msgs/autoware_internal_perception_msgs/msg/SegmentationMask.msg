std_msgs/Header header

# It represent map between instance segmentation mask and labels
# The pixel intensities in mask's starts with 1 (pixel value 0 represent pixel is not belong any object)
#     `classification[0]` returns the `label_id` for pixels with an intensity value of `1` in the mask.
#     `classification[1]` returns the `label_id` for pixels with an intensity value of `2` in the mask.
#     ...
# The relationship between pixels and label IDs continues in this way.
autoware_perception_msgs/ObjectClassification[] classification

sensor_msgs/Image image # Mask image
