# autoware_adapi_v1_msgs

## ResponseStatus

This message is a response status commonly used in the service type API. Each API can define its own status codes.
The status codes are primarily used to indicate the error cause, such as invalid parameter and timeout.
If the API succeeds, set success to true, code to zero, and message to the empty string.
Alternatively, codes and messages can be used for warnings or additional information.
If the API fails, set success to false, code to the related status code, and message to the information.
The status code zero is reserved for success. The status code 50000 or over are also reserved for typical cases.

## Routing

The routing service support two formats. One uses pose and the other uses map dependent data directly.
The body part of the route message is optional, since the route does not exist when it is cleared by the service.
[See also routing API][api-routing].

## Localization

The initialization initialization state does not reflect localization errors. Use diagnostics for that purpose.
[See also localization API][api-localization].

<!-- link -->

[api-localization]: https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-interfaces/ad-api/list/api/localization/
[api-routing]: https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-interfaces/ad-api/list/api/routing/
