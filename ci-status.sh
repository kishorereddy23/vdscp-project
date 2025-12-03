#!/usr/bin/env bash

# --- CONFIG ---
GITLAB_URL="https://eisgit.eit.uni-kl.de"
DEPLOY_TOKEN="glpat-sjy8BP8983GbV-msd4GLTG86MQp1OjM5CA.01.0y1q13cz5"
PROJECT_PATH="extern/vdscp/vdscp2526/vdscp11"  

POLL_INTERVAL=10  # seconds to wait before checking status again

function error_exit {
    echo "ERROR: $1" >&2
    exit 1
}

function urlencode() {
    local string="${1}"
    echo -n "$string" | jq -s -R -r @uri
}

# Get numeric project ID
PROJECT_ID=$(curl --silent --header "PRIVATE-TOKEN: $DEPLOY_TOKEN" \
    "$GITLAB_URL/api/v4/projects/$(urlencode "$PROJECT_PATH")" | jq '.id') || error_exit "Could not fetch project ID"


# Get latest pipeline for current branch
BRANCH=$(git rev-parse --abbrev-ref HEAD)

while true; do
    LATEST_PIPELINE_JSON=$(curl --silent --header "PRIVATE-TOKEN: $DEPLOY_TOKEN" \
        "$GITLAB_URL/api/v4/projects/$PROJECT_ID/pipelines?ref=$BRANCH&per_page=1") || error_exit "Could not fetch pipelines"

    PIPELINE_ID=$(echo "$LATEST_PIPELINE_JSON" | jq '.[0].id') || error_exit "No pipelines found"
    PIPELINE_STATUS=$(echo "$LATEST_PIPELINE_JSON" | jq -r '.[0].status')

    echo "Latest pipeline ID: $PIPELINE_ID, status: $PIPELINE_STATUS"

    if [[ "$PIPELINE_STATUS" != "running" && "$PIPELINE_STATUS" != "pending" ]]; then
        break
    fi

    echo "Pipeline is still $PIPELINE_STATUS. Waiting $POLL_INTERVAL seconds before checking again..."
    sleep $POLL_INTERVAL
done

# Optional: show more details
read -p "Show more details about this pipeline? (y/n) " answer
if [[ "$answer" =~ ^[Yy]$ ]]; then
    PIPELINE_JSON=$(curl --silent --header "PRIVATE-TOKEN: $DEPLOY_TOKEN" \
        "$GITLAB_URL/api/v4/projects/$PROJECT_ID/pipelines/$PIPELINE_ID") || error_exit "Failed to fetch pipeline details"
    echo "$PIPELINE_JSON" | jq
fi

# Optional: download logs for all jobs
read -p "Download job logs for this pipeline? (y/n) " answer
if [[ "$answer" =~ ^[Yy]$ ]]; then
    JOBS_JSON=$(curl --silent --header "PRIVATE-TOKEN: $DEPLOY_TOKEN" \
        "$GITLAB_URL/api/v4/projects/$PROJECT_ID/pipelines/$PIPELINE_ID/jobs") || error_exit "Failed to fetch jobs"

    echo "$JOBS_JSON" | jq -c '.[]' | while read -r job; do
        JOB_ID=$(echo "$job" | jq '.id')
        JOB_NAME=$(echo "$job" | jq -r '.name')
        JOB_STATUS=$(echo "$job" | jq -r '.status')

        echo "Fetching log for job '$JOB_NAME' (ID: $JOB_ID, status: $JOB_STATUS)..."

        curl --silent --header "PRIVATE-TOKEN: $DEPLOY_TOKEN" \
            "$GITLAB_URL/api/v4/projects/$PROJECT_ID/jobs/$JOB_ID/trace" \
            -o "job-${JOB_NAME}-${JOB_ID}.log" || echo "Failed to fetch log for $JOB_NAME"

        echo "Saved log to job-${JOB_NAME}-${JOB_ID}.log"
    done
fi

