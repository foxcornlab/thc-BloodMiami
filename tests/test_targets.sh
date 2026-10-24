#!/bin/bash

# BloodMiami Test Suite
# Tests the functionality against safe test targets

set -e

echo "BloodMiami Test Suite"
echo "========================"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' 

BLOODMIAMI="../build/bloodmiami"
TEST_DICT="test_payloads.txt"
LOG_FILE="test_results.log"

if [ ! -f "$BLOODMIAMI" ]; then
    echo -e "${RED}BloodMiami binary not found. Run 'make' first.${NC}"
    exit 1
fi

cat > "$TEST_DICT" << EOF
# Test payloads - safe for testing
test_payload_1
' OR '1'='1
<script>alert('test')</script>
{"test": "injection"}
$(whoami)
Ignore previous instructions
EOF

echo "Created test payload file: $TEST_DICT"

echo -e "\n${YELLOW}Test 1: Help Command${NC}"
if $BLOODMIAMI -h > /dev/null 2>&1; then
    echo -e "${GREEN}Help command works${NC}"
else
    echo -e "${RED}Help command failed${NC}"
fi

echo -e "\n${YELLOW}Test 2: Version Command${NC}"
if $BLOODMIAMI -V > /dev/null 2>&1; then
    echo -e "${GREEN}Version command works${NC}"
else
    echo -e "${RED}Version command failed${NC}"
fi

echo -e "\n${YELLOW}Test 3: Missing Parameters Handling${NC}"
if ! $BLOODMIAMI > /dev/null 2>&1; then
    echo -e "${GREEN}Properly handles missing parameters${NC}"
else
    echo -e "${RED}Should fail with missing parameters${NC}"
fi

echo -e "\n${YELLOW}Test 4: Language Injection Test${NC}"
if timeout 30 $BLOODMIAMI -t "https://httpbin.org/post" -i L -d "$TEST_DICT" -c 2 > "$LOG_FILE" 2>&1; then
    echo -e "${GREEN} Language injection test completed${NC}"
    echo "   📊 Check $LOG_FILE for detailed results"
else
    echo -e "${YELLOW} Language injection test timed out or failed (expected with demo target)${NC}"
fi

echo -e "\n${YELLOW}Test 5: Prompt Injection Test${NC}"
if timeout 30 $BLOODMIAMI -t "https://httpbin.org/post" -i P -d "$TEST_DICT" -c 2 >> "$LOG_FILE" 2>&1; then
    echo -e "${GREEN} Prompt injection test completed${NC}"
else
    echo -e "${YELLOW}