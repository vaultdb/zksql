gcloud compute instances create bob1 \
    --project=tts-datagov \
    --zone=us-west3-a \
    --machine-type=e2-medium \
    --network-interface=stack-type=IPV4_ONLY,subnet=default,no-address \
    --maintenance-policy=MIGRATE \
    --provisioning-model=STANDARD \
    --no-service-account \
    --no-scopes \
    --tags=http-server,https-server \
    --create-disk=auto-delete=yes,boot=yes,device-name=bob,image=projects/ubuntu-os-cloud/global/images/ubuntu-2004-focal-v20231025,mode=rw,size=40,type=projects/tts-datagov/zones/us-west3-a/diskTypes/pd-balanced \
    --no-shielded-secure-boot \
    --shielded-vtpm \
    --shielded-integrity-monitoring \
    --labels=goog-ec-src=vm_add-gcloud \
    --reservation-affinity=any

gcloud compute instances create rob1 \
    --project=tts-datagov \
    --zone=us-west3-a \
    --machine-type=e2-medium \
    --network-interface=stack-type=IPV4_ONLY,subnet=default,no-address \
    --maintenance-policy=MIGRATE \
    --provisioning-model=STANDARD \
    --no-service-account \
    --no-scopes \
    --tags=http-server,https-server \
    --create-disk=auto-delete=yes,boot=yes,device-name=bob,image=projects/ubuntu-os-cloud/global/images/ubuntu-2004-focal-v20231025,mode=rw,size=40,type=projects/tts-datagov/zones/us-west3-a/diskTypes/pd-balanced \
    --no-shielded-secure-boot \
    --shielded-vtpm \
    --shielded-integrity-monitoring \
    --labels=goog-ec-src=vm_add-gcloud \
    --reservation-affinity=any