#include "swimps-test.h"
#include "swimps-container.h"

SCENARIO("swimps::container::span", "[swimps-container]") {
    GIVEN("A target buffer of 7 bytes, where each byte is initialised as its index.") {
        char targetBuffer[] = { 0, 1, 2, 3, 4, 5, 6 };
        static_assert(sizeof targetBuffer == 7);

        WHEN("A span is created to miss the first and last bytes.") {
            auto span = swimps::container::Span(targetBuffer + 1, 5);

            THEN("The value at each index is offset by 1.") {
                REQUIRE(span[0] == 1);
                REQUIRE(span[1] == 2);
                REQUIRE(span[2] == 3);
                REQUIRE(span[3] == 4);
                REQUIRE(span[4] == 5);
            }

            THEN("The remaining size of the span is 5") {
                REQUIRE(span.remaining_size() == 5);
            }

            THEN("The target buffer is unchanged.") {
                REQUIRE(targetBuffer[0] == 0);
                REQUIRE(targetBuffer[1] == 1);
                REQUIRE(targetBuffer[2] == 2);
                REQUIRE(targetBuffer[3] == 3);
                REQUIRE(targetBuffer[4] == 4);
                REQUIRE(targetBuffer[5] == 5);
                REQUIRE(targetBuffer[6] == 6);
            }

            AND_WHEN("Each byte in the span has a unique value written to it.") {
                span[0] = 42;
                span[1] = 17;
                span[2] = 50;
                span[3] = 11;
                span[4] = 99;

                THEN("The target buffer is changed at an offset of 1.") {
                    REQUIRE(targetBuffer[1] == 42);
                    REQUIRE(targetBuffer[2] == 17);
                    REQUIRE(targetBuffer[3] == 50);
                    REQUIRE(targetBuffer[4] == 11);
                    REQUIRE(targetBuffer[5] == 99);
                }

                THEN("Reading back from the span gives the assigned values.") {
                    REQUIRE(span[0] == 42);
                    REQUIRE(span[1] == 17);
                    REQUIRE(span[2] == 50);
                    REQUIRE(span[3] == 11);
                    REQUIRE(span[4] == 99);
                }

                THEN("The values outside the span are unchanged.") {
                    REQUIRE(targetBuffer[0] == 0);
                    REQUIRE(targetBuffer[6] == 6);
                }

                AND_WHEN("The post-increment operator is called.") {
                    {
                        const auto returnedSpan = span++;

                        THEN("The returned value hasn't changed; this is *post* increment.") {
                            REQUIRE(returnedSpan.remaining_size() == 5);
                            REQUIRE(returnedSpan[0] == 42);
                            REQUIRE(returnedSpan[1] == 17);
                            REQUIRE(returnedSpan[2] == 50);
                            REQUIRE(returnedSpan[3] == 11);
                            REQUIRE(returnedSpan[4] == 99);
                        }
                    }

                    THEN("The remaining size goes down by 1.") {
                        REQUIRE(span.remaining_size() == 4);
                    }

                    THEN("The span now reads at an offset of 2.") {
                        REQUIRE(span[0] == 17);
                        REQUIRE(span[1] == 50);
                        REQUIRE(span[2] == 11);
                        REQUIRE(span[3] == 99);
                    }

                    THEN("The target buffer is unchanged.") {
                        REQUIRE(targetBuffer[0] == 0);  
                        REQUIRE(targetBuffer[1] == 42);
                        REQUIRE(targetBuffer[2] == 17);
                        REQUIRE(targetBuffer[3] == 50);
                        REQUIRE(targetBuffer[4] == 11);
                        REQUIRE(targetBuffer[5] == 99);
                        REQUIRE(targetBuffer[6] == 6);
                    }

                    AND_WHEN("The pre-increment operator is called.") {
                        {
                            const auto returnedSpan = ++span;

                            THEN("The returned and span is equivalent; this is *pre* increment.") {
                                REQUIRE(returnedSpan == span);
                            }
                        }

                        THEN("The remaining size goes down by 1.") {
                            REQUIRE(span.remaining_size() == 3);
                        }

                        AND_WHEN("Each valid index is written to.") {
                            span[0] = 81;
                            span[1] = 82;
                            span[2] = 83;

                            THEN("Reading back from the span gives the assigned values.") {
                                REQUIRE(span[0] == 81);
                                REQUIRE(span[1] == 82);
                                REQUIRE(span[2] == 83);
                            }

                            THEN("The target buffer is changed at an offset of 3.") {
                                REQUIRE(targetBuffer[3] == 81);  
                                REQUIRE(targetBuffer[4] == 82);
                                REQUIRE(targetBuffer[5] == 83);
                            }

                            THEN("The values outside the span are unchanged.") {
                                REQUIRE(targetBuffer[0] == 0);  
                                REQUIRE(targetBuffer[1] == 42);
                                REQUIRE(targetBuffer[2] == 17);
                                REQUIRE(targetBuffer[6] == 6);
                            }

                            AND_WHEN("The post-decrement operator is called.") {
                                {
                                    const auto returnedSpan = span--;

                                    THEN("The return value hasn't changed; this is *post* decrement.") {
                                        REQUIRE(returnedSpan.remaining_size() == 3);
                                        REQUIRE(returnedSpan[0] == 81);
                                        REQUIRE(returnedSpan[1] == 82);
                                        REQUIRE(returnedSpan[2] == 83);
                                    }
                                }

                                THEN("The remaining size increases by 1.") {
                                    REQUIRE(span.remaining_size() == 4);
                                }

                                THEN("The target buffer is unchanged.") {
                                    REQUIRE(targetBuffer[0] == 0);  
                                    REQUIRE(targetBuffer[1] == 42);
                                    REQUIRE(targetBuffer[2] == 17);
                                    REQUIRE(targetBuffer[3] == 81);  
                                    REQUIRE(targetBuffer[4] == 82);
                                    REQUIRE(targetBuffer[5] == 83);
                                    REQUIRE(targetBuffer[6] == 6);
                                }

                                THEN("The span reads at an offset of 2.") {
                                    REQUIRE(span[0] == 17);
                                    REQUIRE(span[1] == 81);  
                                    REQUIRE(span[2] == 82);
                                    REQUIRE(span[3] == 83);
                                    REQUIRE(span[4] == 6);
                                }

                                AND_WHEN("The pre-decrement operator is called.") {
                                    {
                                        const auto returnedSpan = --span;

                                        THEN("The returned and new span are equivalent; this is *pre* decrement.") {
                                            REQUIRE(returnedSpan == span);
                                        }
                                    }

                                    THEN("The remaining size increases by 1.") {
                                        REQUIRE(span.remaining_size() == 5);
                                    }

                                    THEN("The span reads at an offset of 1.") {
                                        REQUIRE(span[0] == 42);
                                        REQUIRE(span[1] == 17);
                                        REQUIRE(span[2] == 81);  
                                        REQUIRE(span[3] == 82);
                                        REQUIRE(span[4] == 83);
                                    }

                                    THEN("The target buffer is unchanged.") {
                                        REQUIRE(targetBuffer[0] == 0);  
                                        REQUIRE(targetBuffer[1] == 42);
                                        REQUIRE(targetBuffer[2] == 17);
                                        REQUIRE(targetBuffer[3] == 81);  
                                        REQUIRE(targetBuffer[4] == 82);
                                        REQUIRE(targetBuffer[5] == 83);
                                        REQUIRE(targetBuffer[6] == 6);
                                    }

                                    AND_WHEN("Each valid index is written to.") {
                                        span[0] = 10;
                                        span[1] = 11;
                                        span[2] = 12;
                                        span[3] = 13;
                                        span[4] = 14;

                                        THEN("The target buffer is changed at an offset of 1.") {
                                            REQUIRE(targetBuffer[1] == 10);
                                            REQUIRE(targetBuffer[2] == 11);
                                            REQUIRE(targetBuffer[3] == 12);  
                                            REQUIRE(targetBuffer[4] == 13);
                                            REQUIRE(targetBuffer[5] == 14);
                                        }

                                        THEN("The values outside of the target buffer are unchanged.") {
                                            REQUIRE(targetBuffer[0] == 0);  
                                            REQUIRE(targetBuffer[6] == 6);
                                        } 

                                        THEN("Reading back from the span gives the assigned values.") {
                                            REQUIRE(span[0] == 10);
                                            REQUIRE(span[1] == 11);
                                            REQUIRE(span[2] == 12);
                                            REQUIRE(span[3] == 13);
                                            REQUIRE(span[4] == 14);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
