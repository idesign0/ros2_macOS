# autoware_map_msgs

## AreaInfo.msg

The message represents an area information. This is intended to be used as a query for partial / differential map loading (see `GetPartialPointCloudMap.srv` and `GetDifferentialPointCloudMap.srv` section).

## PointCloudMapCellWithID.msg

The message contains a pointcloud data attached with an ID.

## PointCloudMapCellMetaDataWithID.msg

The message contains a pointcloud meta data attached with an ID. These IDs are intended to be used as a query for selected PCD map loading (see `GetSelectedPointCloudMap.srv` section).

## MapProjectorInfo.msg

The message contains the information required to project global coordinates to local coordinates used by Autoware, which includes the name of the projection method and the parameters for the projection.
For further information, please refer to the readme of [map_projection_loader](https://github.com/autowarefoundation/autoware.universe/blob/main/map/autoware_map_projection_loader/README.md) in Autoware Universe.

## GetPartialPointCloudMap.srv

Given an area query (`AreaInfo`), the response is expected to contain the PCD maps (each of which attached with unique ID) whose area overlaps with the query.

<img src="./media/partial_area_loading.png" alt="drawing" width="400"/>

## GetDifferentialPointCloudMap.srv

Given an area query and the IDs that the client node already has, the response is expected to contain the PCD maps (each of which attached with unique ID) that...

- overlaps with the area query
- is not possessed by the client node

Let $X_0$ be a set of PCD map ID that the client node has, $X_1$ be a set of PCD map ID that overlaps with the area query, ${\rm pcd}(id)$ be a function that returns PCD data that corresponds to ID $id$. In this case, the response would be

- `loaded_pcds`: $\lbrace [id,{\rm pcd}(id)]~|~id \in X_1 \backslash X_0 \rbrace$
- `ids_to_remove`: $\lbrace id~|~id \in X_0 \backslash X_1 \rbrace$

( $x \in A\backslash B \iff x \in A \wedge x \notin B$ )

<img src="./media/differential_area_loading.gif" alt="drawing" width="400"/>

## GetSelectedPointCloudMap.srv

Given IDs query, the response is expected to contain the PCD maps (each of which attached with unique ID) specified by query. Before using this interface, the client is expected to receive the `PointCloudMapCellMetaDataWithID.msg` metadata to retrieve information about IDs.
