gcloud compute addresses create bob-ip-2 --project=tts-datagov --region=us-west3
export vm_name=bob2 &&
gcloud compute instances create $vm_name \
    --project=tts-datagov \
    --zone=us-west3-a \
    --machine-type=e2-standard-2 \
    --network-interface=address=34.106.0.144,network-tier=PREMIUM,stack-type=IPV4_ONLY,subnet=default \
    --maintenance-policy=MIGRATE \
    --provisioning-model=STANDARD \
    --service-account=storage-admin@tts-datagov.iam.gserviceaccount.com \
    --scopes=https://www.googleapis.com/auth/cloud-platform \
    --tags=swarm \
    --create-disk=auto-delete=yes,boot=yes,device-name=alice2,image=projects/debian-cloud/global/images/debian-11-bullseye-v20230615,mode=rw,size=100,type=projects/tts-datagov/zones/us-west3-a/diskTypes/pd-balanced \
    --no-shielded-secure-boot \
    --shielded-vtpm \
    --shielded-integrity-monitoring \
    --labels=goog-ec-src=vm_add-gcloud \
    --reservation-affinity=any