#ifdef DFTFE_MINIMAL_COMPILE
template class kohnShamDFTOperatorClass<2, 2, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<3, 3, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 4, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 5, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 6, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 7, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 9, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 7, dftfe::utils::MemorySpace::HOST>;

template class kohnShamDFTOperatorClass<2,
                                        2,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<3,
                                        3,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        4,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        5,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        6,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        9,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
#else
template class kohnShamDFTOperatorClass<1, 1, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<1, 2, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<2, 2, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<2, 3, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<2, 4, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<3, 3, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<3, 4, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<3, 5, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<3, 6, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 4, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 5, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 6, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 7, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<4, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 5, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 6, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 7, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 9, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<5, 10, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 6, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 7, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 9, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 10, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 11, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<6, 12, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 7, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 9, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 10, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 11, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 12, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 13, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<7, 14, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 8, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 9, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 10, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 11, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 12, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 13, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 14, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 15, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<8, 16, dftfe::utils::MemorySpace::HOST>;
template class kohnShamDFTOperatorClass<1,
                                        1,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<1,
                                        2,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<2,
                                        2,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<2,
                                        3,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<2,
                                        4,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<3,
                                        3,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<3,
                                        4,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<3,
                                        5,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<3,
                                        6,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        4,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        5,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        6,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<4,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        5,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        6,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        9,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<5,
                                        10,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        6,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        9,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        10,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        11,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<6,
                                        12,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        7,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        9,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        10,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        11,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        12,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        13,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<7,
                                        14,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        8,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        9,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        10,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        11,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        12,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        13,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        14,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        15,
                                        dftfe::utils::MemorySpace::DEVICE>;
template class kohnShamDFTOperatorClass<8,
                                        16,
                                        dftfe::utils::MemorySpace::DEVICE>;
#endif
